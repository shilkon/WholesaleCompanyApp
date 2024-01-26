#ifndef SYSTEM_H
#define SYSTEM_H

#include "gui.h"
#include "database.h"
#include "account.h"

#include <boost/program_options/parsers.hpp>

#include <set>
#include <string>
#include <exception> 

namespace po = boost::program_options;

namespace wholesaleCompanySystem
{
    struct WCSystem
    {
        UAccount account{};
        DB::UConnection connection{};
        DB::UDatabase database{};
    };

    inline constexpr std::string_view INI_FILE_NAME{ "config.ini" };

    po::parsed_options getConfigOptions();
    std::string getConnectionString(const po::parsed_options& parsedOptions);
}

#endif
