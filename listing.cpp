#include "listing.hpp"

#include <boost/filesystem.hpp>

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

namespace RemoteBuild
{
//#####################################################################################################################
    std::string makeHash(std::string const& fileName)
    {
        using namespace CryptoPP;

        SHA256 hash;
        std::string hashString;
        FileSource(
            fileName.c_str(),
            true,
            new HashFilter(
                hash,
                new HexEncoder(
                    new StringSink(hashString),
                    true
                )
            )
        );

        return hashString;
    }
//---------------------------------------------------------------------------------------------------------------------
    bool checkMask(std::string const& p, std::string const& mask)
    {
        std::function <bool(const char*, const char*)> match;

        match = [&match](char const *needle, char const *haystack) -> bool
        {
            for (; *needle != '\0'; ++needle)
            {
                switch (*needle)
                {
                    case '?':
                        if (*haystack == '\0')
                            return false;
                        ++haystack;
                        break;
                    case '*':
                    {
                        if (needle[1] == '\0')
                            return true;
                        size_t max = strlen(haystack);
                        for (size_t i = 0; i < max; i++)
                            if (match(needle + 1, haystack + i))
                                return true;
                        return false;
                    }
                    default:
                        if (*haystack != *needle)
                            return false;
                        ++haystack;
                }
            }
            return *haystack == '\0';
        };

        return match(mask.c_str(), p.c_str());
    }
//#####################################################################################################################
    DirectoryListing makeListing(std::string const& root, bool directories, std::string const& globber)
    {
        DirectoryListing result;
        result.root = root;
        result.filter = globber;

        using namespace boost::filesystem;
        recursive_directory_iterator iter(root), end;
        for (; iter != end; ++iter)
        {
            if (!directories && !is_regular_file(iter->status()))
                continue;
            else if (directories && !is_directory(iter->status()))
                continue;

            auto entry = relative(iter->path(), root).string();
            std::replace(entry.begin(), entry.end(), '\\', '/');

            if (!globber.empty() && !checkMask(entry, globber))
                continue;

            if (!directories)
                result.entriesWithHash.emplace_back(
                    entry,
                    makeHash(iter->path().string())
                );
            else
                result.entriesWithHash.emplace_back(
                    entry,
                    ""
                );
        }

        return result;
    }
//---------------------------------------------------------------------------------------------------------------------
    DirectoryListing addHashes(std::string const& root, std::vector <boost::filesystem::path> fileList)
    {
        using namespace boost::filesystem;

        DirectoryListing result;
        result.root = root;
        for (auto const& entry : fileList)
        {
            result.entriesWithHash.emplace_back(
                entry.string(),
                makeHash((path{root} / entry).string())
            );
        }
        return result;
    }
//---------------------------------------------------------------------------------------------------------------------
    DifferenceActions getDifference(DirectoryListing const& serverListing, DirectoryListing const& clientListing)
    {
        std::vector <std::string> difference;

        auto getLeftDifference = [](DirectoryListing const& source, DirectoryListing const& destination, bool checkHash)
        {
            std::vector <std::string> difference;

            for (auto const& searcher : source.entriesWithHash)
            {
                bool found = false;
                for (auto const& matcher : destination.entriesWithHash)
                {
                    auto mfile = matcher.entry;
                    auto sfile = searcher.entry;
                    if (matcher.entry == searcher.entry)
                    {
                        if (checkHash && matcher.hash != searcher.hash)
                            difference.push_back(searcher.entry);
                        found = true;
                        break;
                    }
                }
                if (!found)
                    difference.push_back(searcher.entry);
            }

            return difference;
        };

        return {
            getLeftDifference(clientListing, serverListing, true),
            getLeftDifference(serverListing, clientListing, false)
        };
    }
//#####################################################################################################################
}
