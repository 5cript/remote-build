#pragma once

#include "../listing.hpp"

#include <attendee/attendee/forward.hpp>

#include <string>
#include <memory>

class CommunicatorEx
{
public:
    CommunicatorEx(std::string const& remote, std::string const& remotePathSuf);
    ~CommunicatorEx();

    bool authenticate(std::string const& user, std::string const& password);
    void uploadFile(std::string const& local, std::string const& remote);
    void makeDirectory(std::string const& remote);
    std::string makeRequest(std::string const& command);
    void remove(std::string const& remote);
    RemoteBuild::DirectoryListing getListing(std::string const& mask);

private:
    template <typename T>
    void testForErrors(T const& res) const
    {
        if (res.result() != 0)
            throw std::runtime_error{std::string{"curl error "} + curl_easy_strerror(res.result())};
    }

private:
    std::string remoteAddr_;
    std::string remoteServer_;
    std::string authToken_;
    std::unique_ptr <attendee::system_context> context_;
};
