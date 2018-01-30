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
//#####################################################################################################################
    DirectoryListing makeListing(std::string const& root, std::string const& extensionWhiteSelect)
    {
        using namespace boost::filesystem;

        DirectoryListing result;
        result.root = root;
        result.filter = extensionWhiteSelect;

        recursive_directory_iterator iter(root), end;
        for (; iter != end; ++iter)
        {
            if (!is_regular_file(iter->status()))
                continue;

            if (!extensionWhiteSelect.empty() && iter->path().extension().string() != extensionWhiteSelect)
                continue;

            auto file = relative(iter->path(), root).string();
            std::replace(file.begin(), file.end(), '\\', '/');
            result.filesWithHash.emplace_back(
                file,
                makeHash(iter->path().string())
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
        for (auto const& file : fileList)
        {
            result.filesWithHash.emplace_back(
                file.string(),
                makeHash((path{root} / file).string())
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

            for (auto const& searcher : source.filesWithHash)
            {
                bool found = false;
                for (auto const& matcher : destination.filesWithHash)
                {
                    auto mfile = matcher.file;
                    auto sfile = searcher.file;
                    if (matcher.file == searcher.file)
                    {
                        if (checkHash && matcher.sha256 != searcher.sha256)
                            difference.push_back(searcher.file);
                        found = true;
                        break;
                    }
                }
                if (!found)
                    difference.push_back(searcher.file);
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
