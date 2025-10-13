#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <ctime>
#include <forward_list>

#include "extra_sources.hpp"

#define CFG_DEFAULT_NUM_VALUE   0u
#define CFG_DEFAULT_STR_VALUE   ""

extern const fs::path gkConfigPath;

struct RgcConfig {
    std::string dlcRootPath;
    std::string v2ipRootPath;
    std::string geoMgrBinaryPath;

    std::time_t refilterTime;
    std::time_t v2rayTime;
    std::time_t ruadlistTime;

    std::string apiToken;

    std::forward_list<ExtraSource> extraSources;

    std::string whitelistPath;
};

bool writeConfig(const RgcConfig& config);

bool readConfig(RgcConfig& config);

#endif // CONFIG_HPP
