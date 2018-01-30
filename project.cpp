#include "project.hpp"

#include "listing.hpp"

#include <iostream>
#include <algorithm>
#include <iterator>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

using namespace std::string_literals;
//#####################################################################################################################
Project::Project(std::string rootDir,
                 std::string id,
                 std::string remoteServer,
                 std::string const& user,
                 std::string const& password,
                 bool ignoreUploadError)
    : rootDir_{rootDir}
    , id_{std::move(id)}
    , ignoreUploadError_{ignoreUploadError}
    , com_{remoteServer, id_}
{
    com_.initialize();
    if (!com_.authenticate(user, password))
        throw std::runtime_error("authentication failed");
}
//---------------------------------------------------------------------------------------------------------------------
Project::~Project()
{
    com_.cleanup();
}
//---------------------------------------------------------------------------------------------------------------------
void Project::createDirectoryStructure(std::vector <std::string> const& dirFilter, bool updatedOnly)
{
    Globber glob(rootDir_, true);
    glob.setDirectoryBlackList(dirFilter);
    auto directories = glob.globRecursive("*");
    for (auto const& i : directories)
    {
        auto path = i.string();
#ifdef _WIN32
        auto attributes = GetFileAttributes(path.c_str());
        if (updatedOnly && (attributes & FILE_ATTRIBUTE_ARCHIVE))
        {
            com_.makeDirectory(path);
            SetFileAttributes(path.c_str(), attributes & ~FILE_ATTRIBUTE_ARCHIVE);
        }
        else if (!updatedOnly)
            com_.makeDirectory(path);
#else
        com_.makeDirectory(path);
#endif // _WIN32
    }
}
//---------------------------------------------------------------------------------------------------------------------
void Project::unidirectional_sychronize(
    std::vector <std::string> const& fileFilter,
    std::vector <std::string> const& dirFilter,
    bool updatedOnly,
    std::vector <std::string> const& mask,
    bool diffOnly,
    bool killServerFiles
)
{
    Globber glob(rootDir_);
    glob.setBlackList(fileFilter);
    glob.setDirectoryBlackList(dirFilter);

    for (auto const& globberExpression : mask)
    {
        auto serverListing = com_.getListing(globberExpression);
        auto files = glob.globRecursive(globberExpression);
        auto clientListing = RemoteBuild::addHashes(rootDir_, files);

        std::vector <std::string> diff;
        if (updatedOnly)
        {
            auto actions = RemoteBuild::getDifference(serverListing, clientListing);
            diff = actions.uploadList;

            if (!actions.deleteList.empty())
            {
                if (killServerFiles)
                {
                    std::cout << "The following files are present on server and will be deleted.\n";
                    for (auto const& i : actions.deleteList)
                        com_.remove(i);
                }
                else
                    std::cout << "WARNING: The following files are present on server, but not client.\n";

                for (auto const& i : actions.deleteList)
                    std::cout << "\t" << i << "\n";
            }
        }
        else
        {
            std::transform(std::begin(files), std::end(files), std::back_inserter(diff), [](auto const& p)
            {
                return p.string();
            });
        }

        if (diffOnly)
        {
            std::cout << "Files for Mask " << globberExpression << ":\n";
            for (auto const& i : diff)
                std::cout << "\t" << i << "\n";
            continue;
        }

        for (auto const& i : diff)
        {
            auto upload = [&]()
            {
                if (!diffOnly)
                {
                    auto local = rootDir_ + "/" + i;
                    com_.uploadFile(local, i);
                }
            };

            if (ignoreUploadError_)
            {
                try
                {
                    upload();
                }
                catch (std::exception const& exc)
                {
                    std::cerr << exc.what() << "\n";
                    std::cerr << "ignoring error, continue\n";
                }
            }
            else
            {
                upload();
            }
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------
bool Project::isBuilding()
{
    auto res = com_.makeRequest("running");
    if (res == "1")
        return true;
    if (res == "0")
        return false;

    throw std::runtime_error(("unexpected response from server"s + res).c_str());
}
//---------------------------------------------------------------------------------------------------------------------
int Project::getExitStatus()
{
    auto res = com_.makeRequest("exit_status");
    return std::stol(res);
}
//---------------------------------------------------------------------------------------------------------------------
void Project::build()
{
    com_.makeRequest("build");
}
//---------------------------------------------------------------------------------------------------------------------
void Project::clean()
{
    com_.makeRequest("clean");
}
//#####################################################################################################################
