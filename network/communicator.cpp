#include "communicator.hpp"

#include <curl/curl.h>

#include <memory>
#include <iostream>

using namespace std::string_literals;
//#####################################################################################################################
static size_t read_callback(char* buffer, size_t size, size_t nmemb, void *stream)
{
    UploadContext* uctx = reinterpret_cast <UploadContext*> (stream);

    uctx->reader.read(buffer, size * nmemb);
    auto gcount = uctx->reader.gcount();
    return gcount;
}
//---------------------------------------------------------------------------------------------------------------------
size_t writeFunction(char *ptr, size_t size, size_t nmemb, std::string* data)
{
    if (data != nullptr)
        data->append((char*) ptr, size * nmemb);
    else
        std::cout << std::string{ptr, size * nmemb} << "\n";
    return size * nmemb;
}
//#####################################################################################################################
Communicator::Communicator(std::string const& remote, std::string const& remotePathSuf)
    : remoteServer_{remote + "/" + remotePathSuf}
{
}
//---------------------------------------------------------------------------------------------------------------------
void Communicator::uploadFile(std::string const& local, std::string const& remote)
{
    CURL *curl = curl_easy_init();

    if (curl)
    {
        std::size_t fileSize{0};
        {
            std::ifstream sizeSkim{local, std::ios_base::binary | std::ios_base::ate};
            if (!sizeSkim.good())
            {
                std::cerr << "cannot upload " << local << "\n";
                return;
            }
            fileSize = sizeSkim.tellg();
        }

        UploadContext uctx{std::ifstream{local, std::ios_base::binary}};

        struct curl_slist *list = nullptr;
        list = curl_slist_append(list, "Expect:");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, (remoteServer_ + "/" + remote).c_str());
        curl_easy_setopt(curl, CURLOPT_READDATA, &uctx);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fileSize);

        std::cout << "uploading file: " << local << "\n";
        auto res = curl_easy_perform(curl);

        curl_slist_free_all(list);
        curl_easy_cleanup(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            throw std::runtime_error{"curl error "s + curl_easy_strerror(res)};
    }
}
//---------------------------------------------------------------------------------------------------------------------
void Communicator::makeDirectory(std::string const& remote)
{
    CURL *curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, (remoteServer_ + "_mkdir").c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, remote.c_str());

        std::cout << "making directory: " << remote << "\n";
        auto res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if(res != CURLE_OK)
            throw std::runtime_error{"curl error "s + curl_easy_strerror(res)};
    }
}
//---------------------------------------------------------------------------------------------------------------------
std::string Communicator::makeRequest(std::string const& command)
{
    CURL *curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, (remoteServer_ + "_" + command).c_str());
        std::string response_string;
        std::string header_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
        auto res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if(res != CURLE_OK)
            throw std::runtime_error{"curl error "s + curl_easy_strerror(res)};

        return response_string;
    }
}
//---------------------------------------------------------------------------------------------------------------------
void Communicator::initialize()
{
    curl_global_init(CURL_GLOBAL_ALL);
}
//---------------------------------------------------------------------------------------------------------------------
void Communicator::cleanup()
{
    curl_global_cleanup();
}
//#####################################################################################################################
