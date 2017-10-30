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
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

namespace po = boost::program_options;

int main(int argc, char** argv)
{
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
        ("upload,u", "upload files to host")
        ("build,b", "build stuff on host")
        ("clean,c", "clean stuff on host")
        ("updated-only,o", "clean stuff on host")
        ("ignore-upload-fail,i", "continue, even if a file could not be opened for upload")
        ("timeout,t", po::value<int>(&buildTimeout)->default_value(60), "build wait timeout (seconds)")
        ("local-scripts,l", "perform local post/pre steps. This is an unsecure option for the local PC.")
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
    // PROJECT
    //######################################################################################################

    std::ifstream reader(configPath, std::ios_base::binary);
    if (!reader.good())
    {
        std::cerr << "provide config.json\n";
		std::cerr << "could not open: " << configPath << "\n";
		std::cerr << "(current path: " << boost::filesystem::current_path() << ")\n";
        return 1;
    }
    auto config = loadConfig(reader);

    Project project(config.local, config.id, config.serverAddress, config.user, config.password, ignoreUploadError);

    //######################################################################################################
    // START REQUESTS
    //######################################################################################################

    if (vm.count("local-scripts") && config.localPreUploadSteps)
    {
        std::cout << "running: " << config.localPreUploadSteps.get() << "\n";
        system(config.localPreUploadSteps.get().c_str());
    }

    if (vm.count("clean"))
        project.clean();
    if (vm.count("make-directories"))
    {
        std::vector <std::string> directoryFilter;
        if (config.directoryFilter)
            directoryFilter = config.directoryFilter.get();

        project.createDirectoryStructure(
            directoryFilter,
            vm.count("clean") == 0 && vm.count("updated-only")
        );
    }
    if (true || vm.count("upload"))
    {
        std::vector <std::string> fileFilter;
        std::vector <std::string> dirFilter;
        if (config.fileFilter)
            fileFilter = config.fileFilter.get();
        if (config.directoryFilter)
            dirFilter = config.directoryFilter.get();

        std::vector <std::string> globExpr {"*.?pp"};
        if (config.globExpressions)
            globExpr = config.globExpressions.get();

        project.upload(
            fileFilter,
            dirFilter,
            vm.count("clean") == 0 && vm.count("updated-only"),
            globExpr
        );
    }
    if (vm.count("local-scripts") && config.localPostUploadSteps)
    {
        std::cout << "running: " << config.localPostUploadSteps.get() << "\n";
        system(config.localPostUploadSteps.get().c_str());
    }
    if (vm.count("build"))
    {
        project.build();

        std::cout << "waiting for build to finish...\n";
        std::this_thread::sleep_for(std::chrono::seconds{1});
        for (int i = 0; buildTimeout == 0 || i < buildTimeout; i += 3)
        {
            if (!project.isBuilding())
            {
                std::ofstream writer{config.log, std::ios_base::binary};
                project.saveBuildLog(std::cout, writer);
                return project.getExitStatus();
            }
            std::this_thread::sleep_for(std::chrono::seconds{3});
        }
        std::cout << "waiting for build to finish timed out, here is the build log so far\n";
        std::ofstream writer{config.log, std::ios_base::binary};
        project.saveBuildLog(std::cout, writer);

        return 0;
    }

    }
    catch(std::exception const& exc)
    {
        std::cout << "something failed...\n" << exc.what() << "\n";

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
