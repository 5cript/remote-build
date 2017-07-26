#pragma once

#ifndef Q_MOC_RUN // A Qt workaround, for those of you who use Qt
#   include <SimpleJSON/parse/jsd_fusion_adapted_struct.hpp>
#   include <SimpleJSON/stringify/jss_fusion_adapted_struct.hpp>
#endif

#include <vector>
#include <string>
#include <boost/optional.hpp>

struct Config : public JSON::Stringifiable <Config>
              , public JSON::Parsable <Config>
{
    std::string serverAddress;
    std::string local;
    std::string id;
    std::string log;
    boost::optional <std::vector <std::string>> directoryFilter;
    boost::optional <std::vector <std::string>> fileFilter;
    boost::optional <std::string> globExpression;
};

Config loadConfig(std::istream& json);
void saveConfig(std::ostream& stream, Config const& cfg);

BOOST_FUSION_ADAPT_STRUCT
(
    Config,
    serverAddress, local, id, directoryFilter, fileFilter, globExpression
)
