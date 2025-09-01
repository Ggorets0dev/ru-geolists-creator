#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <ctime>

#include "extra_sources.hpp"

#define OUTPUT_FOLDER_NAME      "output"
#define GEOSITE_FILE_NAME       "geosite.dat"
#define GEOIP_FILE_NAME         "geoip.dat"

#define CFG_DEFAULT_NUM_VALUE   0u
#define CFG_DEFAULT_STR_VALUE   ""

extern const fs::path gkConfigPath;

struct RgcConfig {
    std::string dlcRootPath;
    std::string v2ipRootPath;

    std::time_t refilterTime;
    std::time_t v2rayTime;
    std::time_t ruadlistTime;

    std::string apiToken;

    std::vector<ExtraSource> extraSources;
};

extern bool
writeConfig(const RgcConfig& config);

extern bool
readConfig(RgcConfig& config);

#endif // CONFIG_HPP
