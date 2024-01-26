#include "system.h"

namespace WHCSys = wholesaleCompanySystem;

po::parsed_options WHCSys::getConfigOptions()
{
    po::options_description desc("All options");
    desc.add_options()
        ("dbname", po::value<std::string>())
        ("hostaddr", po::value<std::string>())
        ("user", po::value<std::string>())
        ("password", po::value<std::string>());
    
    return po::parse_config_file(INI_FILE_NAME.data(), desc);
}

std::string WHCSys::getConnectionString(const po::parsed_options& parsedOptions)
{
    std::string connectionString{};
    for (const auto& option : parsedOptions.options)
        connectionString += option.string_key + '=' + option.value[0] + ' ';
    return connectionString;
}
