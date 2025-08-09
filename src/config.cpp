#include "config.hpp"

#define SET_NULL_IF_EMPTY(jsonValue, variable) \
    if (!variable.empty()) { \
        jsonValue = variable; \
    } else { \
        jsonValue = Json::nullValue; \
    }

bool
writeConfig(const RgcConfig& config) {
    bool status;
    Json::Value value;
    Json::Value extraArray(Json::arrayValue);

    value["dlcRootPath"] = config.dlcRootPath;
    value["v2ipRootPath"] = config.v2ipRootPath;

    value["refilterTime"] = config.refilterTime;
    value["v2rayTime"] = config.v2rayTime;

    value["ruadlistTime"] = config.ruadlistTime;

    SET_NULL_IF_EMPTY(value["apiToken"], config.apiToken);

    for (const auto& source : config.extraSources) {
        Json::Value obj;

        obj["type"] = sourceTypeToString(source.type);
        obj["url"] = source.url;
        obj["section"] = source.section;

        extraArray.append(obj);
    }

    value["extra"] = extraArray;

    status = writeJsonToFile(RGC_CONFIG_PATH, value);

    if (!status) {
        LOG_ERROR("Configuration file could not be written due to an error");
    }

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
    config.ruadlistTime = value["v2rayTime"].asInt64();

    config.apiToken = value["apiToken"].asString();

    if (value["extra"].isArray() && !value["extra"].isNull()) {
        const Json::Value& sources = value["extra"];
        for (const auto& source : sources) {
            config.extraSources.push_back(ExtraSource(source));
        }
    }

    return true;
}
