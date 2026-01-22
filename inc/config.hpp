#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <unordered_map>

#include "main_sources.hpp"

#define CFG_DEFAULT_NUM_VALUE   0u
#define CFG_DEFAULT_STR_VALUE   ""

extern const fs::path gkConfigPath;

struct RgcConfig {
    std::string dlcRootPath;
    std::string v2ipRootPath;
    std::string geoMgrBinaryPath;

    std::string apiToken;

    std::unordered_map<std::string, SourcePreset> presets;
    std::unordered_map<SourceObjectId, Source> sources;

    std::string whitelistPath;
    std::string bgpDumpPath;
};

bool writeConfig(const RgcConfig& config);

bool validateConfig(const RgcConfig& config);

bool readConfig(RgcConfig& config);

bool initSoftwareConfig();

const RgcConfig* getCachedConfig();

void setCachedConfig(const RgcConfig& config);

#endif // CONFIG_HPP
