#include "communicator.hpp"

#ifndef Q_MOC_RUN // A Qt workaround, for those of you who use Qt
#   include <SimpleJSON/parse/jsd.hpp>
#   include <SimpleJSON/stringify/jss.hpp>
#endif

#include <boost/algorithm/string.hpp>

#include <attendee/attendee.hpp>
#include <attendee/attendee/source/file_source.hpp>
#include <attendee/attendee/source/string_source.hpp>

using namespace attendee;
using namespace std::string_literals;

//#####################################################################################################################
CommunicatorEx::CommunicatorEx(std::string const& remote, std::string const& remotePathSuf)
    : remoteAddr_{remote + "/"}
    , remoteServer_{remote + "/" + remotePathSuf}
    , authToken_{}
    , context_{new attendee::system_context}
{
}
//---------------------------------------------------------------------------------------------------------------------
CommunicatorEx::~CommunicatorEx() = default;
//---------------------------------------------------------------------------------------------------------------------
bool CommunicatorEx::authenticate(std::string const& user, std::string const& password)
{
    request req;
    auto res = req
        .basic_auth(user, password)
        .get(remoteAddr_ + "authenticate")
        .cookies("")
        .perform()
    ;

    testForErrors(res);

    bool found = false;
    res
        .cookies()
        .for_each([this, &found](auto&& cookie) {
            found = true;
            if (cookie.name == "SESS")
                authToken_ = cookie.value;
        })
    ;
    return found;
}
//---------------------------------------------------------------------------------------------------------------------
void CommunicatorEx::uploadFile(std::string const& local, std::string const& remote)
{
    std::string remoteFixed = remote;
    boost::replace_all(remoteFixed, "\\", "/");

    request req;
    auto res = req
        .cookie_string("SESS="s + authToken_)
        .make_source <attendee::file_source> (local)
        .put(remoteServer_ + "/" + request::url_encode(remoteFixed))
        .perform()
    ;

    testForErrors(res);
}
//---------------------------------------------------------------------------------------------------------------------
void CommunicatorEx::makeDirectory(std::string const& remote)
{
    request req;
    auto res = req
        .cookie_string("SESS="s + authToken_)
        .make_source <attendee::string_source> (remote)
        .post(remoteServer_ + "_mkdir")
        .perform();

    testForErrors(res);
}
//---------------------------------------------------------------------------------------------------------------------
std::string CommunicatorEx::makeRequest(std::string const& command)
{
    std::string response;

    request req;
    auto res = req
        .cookie_string("SESS="s + authToken_)
        .get(remoteServer_ + "_" + command)
        .sink(response)
        .perform()
    ;

    testForErrors(res);

    return response;
}
//---------------------------------------------------------------------------------------------------------------------
void CommunicatorEx::remove(std::string const& remote)
{
    std::string remoteFixed = remote;
    boost::replace_all(remoteFixed, "\\", "/");

    request req;
    auto res = req
        .cookie_string("SESS="s + authToken_)
        .url(remoteServer_ + "/" + remoteFixed)
        .verb("DELETE")
        .perform()
    ;

    testForErrors(res);
}
//---------------------------------------------------------------------------------------------------------------------
RemoteBuild::DirectoryListing CommunicatorEx::getListing(std::string const& mask)
{
    std::string listing;

    request req;
    auto res = req
        .cookie_string("SESS="s + authToken_)
        .sink(listing)
        .get(remoteServer_ + "_listing?filter=" + request::url_encode(mask))
        .perform();
    ;

    testForErrors(res);

    if (listing.empty() || listing == "Not Found" || listing == "Bad Request")
        throw std::runtime_error("Failed to retrieve directory listing.");

    return JSON::make_from_json <RemoteBuild::DirectoryListing> (listing);
}
//#####################################################################################################################
