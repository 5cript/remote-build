#include "lib_remote_build.hpp"
#include "config.hpp"
#include "project.hpp"
//#include "app.hpp"

#include <boost/version.hpp>

#ifndef BOOST_VERSION
#   error "BOOST_VERSION macro undefined, even though boost/version.hpp is included (broken boost installation?)"
#endif // BOOST_VERSION
#if BOOST_VERSION >= 106500
#   include <boost/stacktrace.hpp>
#elif defined(__WIN32)
#   define WINTRACE
#   include "dbg.h"
#endif

#include <boost/program_options.hpp>

#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace po = boost::program_options;

int main(int argc, char** argv)
{
    using namespace RemoteBuild;

    try
    {
        //######################################################################################################
        // PROGRAM OPTIONS
        //######################################################################################################

        std::string configPath;
        int buildTimeout = 60;
        bool ignoreUploadError = false;

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("config,f", po::value<std::string>(&configPath)->default_value("./config.json"), "config to use")
            ("make-directories,m", "create directory on remote host")
            ("synchronize,u", "upload files to host")
            ("build,b", "build stuff on host")
            ("clean,c", "clean stuff on host")
            ("updated-only,o", "only upload files that haven't changed (recommended)")
            ("ignore-upload-fail,i", "continue, even if a file could not be opened for upload")
            ("timeout,t", po::value<int>(&buildTimeout)->default_value(60), "build wait timeout (seconds)")
            ("local-scripts,l", "perform local post/pre steps. This is an unsecure option for the local PC.")
            ("print-diff-only,p", "Only print differences, do not actually upload. needs -u param though.")
            ("kill-server-only,k", "Deletes files that are in the server, but not the client")
        ;

        po::variables_map vm;
        try
        {
            po::store(po::parse_command_line(argc, argv, desc), vm);
        }
        catch (std::exception const& exc)
        {
            std::cerr << "error during command line parsing: " << exc.what() << "\n";
            return 1;
        }
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return 1;
        }
        ignoreUploadError = vm.count("ignore-upload-fail") > 0;

        //######################################################################################################
        // PROJECT CONTEXT
        //######################################################################################################

        auto ctx = makeContext(configPath, ignoreUploadError);

        std::cout << "Server Address: " << ctx.config->serverAddress << "\n";

        //######################################################################################################
        // START REQUESTS
        //######################################################################################################

        if (vm.count("local-scripts"))
            prebuildSteps(ctx);

        if (vm.count("clean"))
            clean(ctx);

        if (vm.count("make-directories"))
            makeDirectories(ctx, !vm.count("updated-only"));

        if (vm.count("synchronize"))
            synchronize(ctx, !static_cast <bool> (vm.count("updated-only")), vm.count("kill-server-only"), vm.count("print-diff-only"));

        if (vm.count("local-scripts"))
            postbuildSteps(ctx);

        if (vm.count("build"))
        {
            build(ctx);
            return waitForBuild(ctx, [](std::string const& str) {std::cout << str;}, buildTimeout);
        }
    }
    catch(std::exception const& exc)
    {
        std::cout << "something failed: " << exc.what() << "\n";

#if BOOST_VERSION >= 106500
        std::cout << boost::stacktrace::stacktrace();
#elif defined(WINTRACE)
        auto trace = dbg::stack_trace();
        for (auto const& i : trace)
        {
            std::cout << "0x" << std::hex << i.address << " " << i.name << " in " << i.file << "(" << i.line << ") in module " << i.module << "\n";
        }
#endif
        return 1;
    }
}
