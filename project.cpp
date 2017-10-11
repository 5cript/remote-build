#include "project.hpp"

#include <iostream>

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
void Project::upload(std::vector <std::string> const& fileFilter,
                     std::vector <std::string> const& dirFilter,
                     bool updatedOnly,
                     std::vector <std::string> const& mask)
{
    Globber glob(rootDir_);
    glob.setBlackList(fileFilter);
    glob.setDirectoryBlackList(dirFilter);

    for (auto const& globberExpression : mask)
    {
        auto files = glob.globRecursive(globberExpression);
        for (auto const& i : files)
        {
            auto upload = [&]()
            {
                auto path = rootDir_ + "/" + i.string();
#ifdef _WIN32
                auto attributes = GetFileAttributes(path.c_str());
                if (updatedOnly && (attributes & FILE_ATTRIBUTE_ARCHIVE))
                {
                    com_.uploadFile(path, i.string());
                    SetFileAttributes(path.c_str(), attributes & ~FILE_ATTRIBUTE_ARCHIVE);
                }
                else if (!updatedOnly)
                    com_.uploadFile(path, i.string());
#else
                com_.uploadFile(path, i.string());
#endif // _WIN32
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
