#pragma once

#include "filesystem/globber.hpp"
#include "network/communicator.hpp"

#include <vector>
#include <string>

namespace RemoteBuild
{
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
        void unidirectional_sychronize(
            std::vector <std::string> const& fileFilter,
            std::vector <std::string> const& dirFilter,
            bool updatedOnly,
            std::vector <std::string> const& mask,
            bool diffOnly,
            bool killServerFiles
        );
        std::pair <std::vector <std::string> /* client upload */, std::vector <std::string> /* server only */> getDiff(
            std::vector <std::string> const& fileFilter,
            std::vector <std::string> const& dirFilter,
            std::vector <std::string> const& mask
        );
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
            stream.write(str.c_str(), str.length());
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

        CommunicatorEx com_;
    };
}
