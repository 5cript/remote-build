#pragma once

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

#include <SimpleJSON/parse/jsd_fusion_adapted_struct.hpp>
#include <SimpleJSON/stringify/jss_fusion_adapted_struct.hpp>

namespace RemoteBuild
{
    struct FileWithHash : public JSON::Stringifiable <FileWithHash>
                        , public JSON::Parsable <FileWithHash>
    {
        std::string file;
        std::string sha256;

        FileWithHash(std::string file = {}, std::string sha256 = {})
            : file{std::move(file)}
            , sha256{std::move(sha256)}
        {
        }
    };

    struct DirectoryListing : public JSON::Stringifiable <DirectoryListing>
                            , public JSON::Parsable <DirectoryListing>
    {
        std::string filter;
        std::string root;
        std::vector <FileWithHash> filesWithHash;
    };

    struct DifferenceActions
    {
        std::vector <std::string> uploadList;
        std::vector <std::string> deleteList;
    };

    /**
     *  @param root Path to recurse through
     *  @param extensionWhiteSelect Will only select files with the given extension. does not filter if empty.
     */
    DirectoryListing makeListing(std::string const& root, std::string const& extensionWhiteSelect = {});
    DirectoryListing addHashes(std::string const& root, std::vector <boost::filesystem::path> fileList);

    DifferenceActions getDifference(DirectoryListing const& serverListing, DirectoryListing const& clientListing);
}

BOOST_FUSION_ADAPT_STRUCT
(
    RemoteBuild::FileWithHash,
    file, sha256
)

BOOST_FUSION_ADAPT_STRUCT
(
    RemoteBuild::DirectoryListing,
    root, filter, filesWithHash
)
