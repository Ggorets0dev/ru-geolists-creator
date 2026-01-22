#include "config.hpp"
#include "log.hpp"
#include "json_io.hpp"

#define SET_NULL_IF_EMPTY(jsonValue, variable) \
    if (!variable.empty()) { \
        jsonValue = variable; \
    } else { \
        jsonValue = Json::nullValue; \
    }

const fs::path gkConfigPath = fs::path(std::getenv("HOME")) / ".config" / "ru-geolists-creator" / "config.json";

static RgcConfig gkConfig;
static bool gIsConfigSet = false;

bool writeConfig(const RgcConfig& config) {
    Json::Value value;

    value["dlcRootPath"] = config.dlcRootPath;
    value["v2ipRootPath"] = config.v2ipRootPath;
    value["geoMgrBinaryPath"] = config.geoMgrBinaryPath;

    SET_NULL_IF_EMPTY(value["apiToken"], config.apiToken);
    SET_NULL_IF_EMPTY(value["whitelistPath"], config.whitelistPath);
    SET_NULL_IF_EMPTY(value["bgpDumpPath"], config.bgpDumpPath);

    Json::Value sourcesArray(Json::arrayValue);
    for (const auto& [id, source] : config.sources) {
        Json::Value sourceObj;

        sourceObj["id"] = source.id; // Используем SourceObjectId
        sourceObj["section"] = source.section;
        sourceObj["url"] = source.url;
        sourceObj["inet_type"] = sourceInetTypeToString(source.inetType);
        sourceObj["storage_type"] = sourceStorageTypeToString(source.storageType);

        if (source.assets) {
            Json::Value assetsArray(Json::arrayValue);
            for (const auto& asset : *source.assets) {
                assetsArray.append(asset);
            }
            sourceObj["assets"] = assetsArray;
        }

        sourcesArray.append(sourceObj);
    }
    value["sources"] = sourcesArray;

    Json::Value presetsArray(Json::arrayValue);
    for (const auto& [label, preset] : config.presets) {
        Json::Value presetObj;
        presetObj["label"] = preset.label;

        Json::Value sourceIdsArray(Json::arrayValue);
        for (const auto& sId : preset.sourceIds) {
            sourceIdsArray.append(sId);
        }
        presetObj["source_ids"] = sourceIdsArray;

        presetsArray.append(presetObj);
    }
    value["presets"] = presetsArray;

    try {
        if (!gkConfigPath.parent_path().empty()) {
            fs::create_directories(gkConfigPath.parent_path());
        }
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("FileSystem error: {}", e.what());
        return false;
    }

    const bool status = writeJsonToFile(gkConfigPath, value);
    if (!status) {
        LOG_ERROR("Configuration file '{}' could not be written", gkConfigPath.string());
    }

    return status;
}

bool readConfig(RgcConfig& config) {
    Json::Value value;

    if (!readJsonFromFile(gkConfigPath, value)) {
        LOG_ERROR(READ_CFG_FAIL_MSG);
        return false;
    }

    config.dlcRootPath = value["dlcRootPath"].asString();
    config.v2ipRootPath = value["v2ipRootPath"].asString();
    config.geoMgrBinaryPath = value["geoMgrBinaryPath"].asString();
    config.apiToken = value["apiToken"].asString();
    config.whitelistPath = value["whitelistPath"].asString();
    config.bgpDumpPath = value["bgpDumpPath"].asString();

    if (value["sources"].isArray()) {
        for (const auto& sourceJson : value["sources"]) {
            try {
                Source source(sourceJson);
                SourceObjectId sId = source.id;

                config.sources.emplace(sId, std::move(source));
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to parse source: " + std::string(e.what()));
                return false;
            }
        }
    }

    if (value["presets"].isArray()) {
        for (const auto& presetJson : value["presets"]) {
            SourcePreset preset;

            preset.label = JsonValidator::getRequired<std::string>(presetJson, "label");

            if (presetJson["source_ids"].isArray()) {
                const Json::Value& ids = presetJson["source_ids"];
                for (const auto& idJson : ids) {
                    auto sId = static_cast<SourceObjectId>(idJson.asUInt());

                    if (config.sources.find(sId) == config.sources.end()) {
                        LOG_WARNING("Preset '{}' references unknown Source ID: {}", preset.label, sId);
                    }

                    preset.sourceIds.emplace_front(sId);
                }
            }

            config.presets.emplace(preset.label, std::move(preset));
        }
    }

    return true;
}

bool validateConfig(const RgcConfig& config) {
    for (const auto&[fst, snd] : config.presets) {
        for (const auto& sourceId : snd.sourceIds) {
            auto sourceIter = config.sources.find(sourceId);

            if (sourceIter == config.sources.end()) {
                return false;
            }
        }
    }

    return true;
}

const RgcConfig* getCachedConfig() {
    if (!gIsConfigSet) {
        throw std::logic_error("getCachedConfig() called when config is not set");
    }

    return &gkConfig;
}

bool initSoftwareConfig() {
    if (!readConfig(gkConfig) || !validateConfig(gkConfig)) {
        LOG_ERROR(READ_CFG_FAIL_MSG);
        return false;
    }

    // Global cache, which will be used in functions
    setCachedConfig(gkConfig);

    return true;
}

void setCachedConfig(const RgcConfig& config) {
    gIsConfigSet = true;
    gkConfig = config;
}