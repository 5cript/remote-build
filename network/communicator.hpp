#pragma once

#include "../listing.hpp"

#include <string>
#include <fstream>

struct UploadContext
{
    std::ifstream reader;

    UploadContext(std::ifstream reader) : reader{std::move(reader)} {}

    UploadContext(UploadContext const&) = delete;
    UploadContext& operator=(UploadContext const&) = delete;

    UploadContext(UploadContext&&) = default;
    UploadContext& operator=(UploadContext&&) = default;
};

class Communicator
{
public:
    Communicator(std::string const& remote, std::string const& remotePathSuf);

    bool authenticate(std::string const& user, std::string const& password);
    void initialize();
    void cleanup();
    void uploadFile(std::string const& local, std::string const& remote);
    void makeDirectory(std::string const& remote);
    void remove(std::string const& remote);
    RemoteBuild::DirectoryListing getListing(std::string const& mask);
    std::string makeRequest(std::string const& command);

private:
    std::string url_encode(std::string const& str);

private:
    std::string remoteAddr_;
    std::string remoteServer_;
    std::string authToken_;
};
