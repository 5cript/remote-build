#pragma once

#include <boost/filesystem.hpp>

#include <string>
#include <vector>

#include <SimpleJSON/parse/jsd_fusion_adapted_struct.hpp>
#include <SimpleJSON/stringify/jss_fusion_adapted_struct.hpp>

namespace RemoteBuild
{
    struct EntryWithHash : public JSON::Stringifiable <EntryWithHash>
                        , public JSON::Parsable <EntryWithHash>
    {
        std::string entry;
        std::string hash;

        EntryWithHash(std::string entry = {}, std::string hash = {})
            : entry{std::move(entry)}
            , hash{std::move(hash)}
        {
        }
    };

    struct DirectoryListing : public JSON::Stringifiable <DirectoryListing>
                            , public JSON::Parsable <DirectoryListing>
    {
        std::string filter;
        std::string root;
        std::vector <EntryWithHash> entriesWithHash;
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
    DirectoryListing makeListing(std::string const& root, bool directories = false, std::string const& extensionWhiteSelect = {});
    DirectoryListing addHashes(std::string const& root, std::vector <boost::filesystem::path> fileList);

    /**
     *  Performs a wildcard match.
     */
    bool checkMask(std::string const& p, std::string const& mask);

    DirectoryListing filterListing(
        DirectoryListing listing,
        std::vector <std::string> const& fileFilter,
        std::vector <std::string> const& dirFilter
    );

    DifferenceActions getDifference(DirectoryListing const& serverListing, DirectoryListing const& clientListing);
}

BOOST_FUSION_ADAPT_STRUCT
(
    RemoteBuild::EntryWithHash,
    entry, hash
)

BOOST_FUSION_ADAPT_STRUCT
(
    RemoteBuild::DirectoryListing,
    root, filter, entriesWithHash
)
