#pragma once

#include "filesystem/globber.hpp"
#include "network/communicator.hpp"

#include <vector>
#include <string>

class Project
{
public:
    Project(std::string rootDir,
            std::string id,
            std::string remoteServer,
            std::string const& user,
            std::string const& password,
            bool ignoreUploadError);
    ~Project();

    Project(Project const&) = delete;
    Project& operator=(Project const&) = delete;

    void createDirectoryStructure(std::vector <std::string> const& dirFilter, bool updatedOnly);
    void upload(std::vector <std::string> const& fileFilter,
                std::vector <std::string> const& dirFilter,
                bool updatedOnly,
                std::vector <std::string> const& mask = {"*.?pp"});
    void build();
    void clean();
    int getExitStatus();

private:
    void putStringToStream(std::string const& str)
    {
        // noop
    }
    template <typename T, typename... Tail>
    void putStringToStream(std::string const& str, T& stream, Tail&... streams)
    {
        stream << str;
        putStringToStream(str, streams...);
    }

public:
    template <typename... T>
    void saveBuildLog(T&... streams)
    {
        std::string log = com_.makeRequest("log");
        putStringToStream(log, streams...);
    }

    bool isBuilding();

private:
    std::string rootDir_;
    std::string id_;
    bool ignoreUploadError_;

    Communicator com_;
};
