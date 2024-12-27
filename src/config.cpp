#include "config.hpp"

bool
writeConfig(const RgcConfig& config) {
    bool status;
    Json::Value value;

    value["dlcRootPath"] = config.dlcRootPath;
    value["v2ipRootPath"] = config.v2ipRootPath;

    value["refilterTime"] = config.refilterTime;
    value["v2rayTime"] = config.v2rayTime;

    if (!config.ruadlistVersion.empty()) {
        value["ruadlistVersion"] = config.ruadlistVersion;
    } else {
        value["ruadlistVersion"] = Json::nullValue;
    }

    status = writeJsonToFile(RGC_CONFIG_PATH, value);

    if (!status) {
        LOG_ERROR("Configuration file could not be written due to an error");
    }

    // TODO: Write extra

    return status;
}

bool
readConfig(RgcConfig& config) {
    bool status;
    Json::Value value;

    status = readJsonFromFile(RGC_CONFIG_PATH, value);

    if (!status) {
        LOG_ERROR("Failed to read configuration file");
        return false;
    }

    config.dlcRootPath = value["dlcRootPath"].asString();
    config.v2ipRootPath = value["v2ipRootPath"].asString();

    config.refilterTime = value["refilterTime"].asInt64();
    config.v2rayTime = value["v2rayTime"].asInt64();
    config.ruadlistVersion = value["ruadlistVersion"].asString();

    // TODO: Read extra

    return true;
}
