#include "config.hpp"
#include "project.hpp"

#include <boost/program_options.hpp>

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

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

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config,i", po::value<std::string>(&configPath)->default_value("./config.json"), "config to use")
        ("make-directories,m", "create directory on remote host")
        ("upload,u", "upload files to host")
        ("build,b", "build stuff on host")
        ("clean,c", "clean stuff on host")
        ("updated-only,o", "clean stuff on host")
        ("timeout,t", po::value<int>(&buildTimeout)->default_value(60), "build wait timeout (seconds)")
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

    //######################################################################################################
    // PROJECT
    //######################################################################################################

    std::ifstream reader(configPath, std::ios_base::binary);
    if (!reader.good())
    {
        std::cerr << "provide config.json\n";
        return 1;
    }
    auto config = loadConfig(reader);

    Project project(config.local, config.id, config.serverAddress);

    //######################################################################################################
    // START REQUESTS
    //######################################################################################################

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
    if (vm.count("upload"))
    {
        std::vector <std::string> fileFilter;
        if (config.fileFilter)
            fileFilter = config.fileFilter.get();
        if (config.directoryFilter)
        {
            auto directoryFilter = config.directoryFilter.get();
            for (auto const& df : directoryFilter)
                fileFilter.push_back(df);
        }

        std::string globExpr = "*.?pp";
        if (config.globExpression)
            globExpr = config.globExpression.get();

        project.upload(
            fileFilter,
            vm.count("clean") == 0 && vm.count("updated-only"),
            globExpr
        );
    }
    if (vm.count("build"))
    {
        project.build();

        std::cout << "waiting for build to finish...\n";
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
    }
}
