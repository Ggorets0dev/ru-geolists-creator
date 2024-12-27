#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <ctime>

#include "json_io.hpp"

#define RGC_CONFIG_PATH     "./rgc_config.json"

struct RgcConfig {
    std::string dlcRootPath;
    std::string v2ipRootPath;

    std::time_t refilterTime;
    std::time_t v2rayTime;
    std::string ruadlistVersion;
};

extern bool
writeConfig(const RgcConfig& config);

extern bool
readConfig(RgcConfig& config);

#endif // CONFIG_HPP
