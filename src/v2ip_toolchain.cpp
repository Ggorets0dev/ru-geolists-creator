#include "v2ip_toolchain.hpp"

#define V2IP_API_LAST_RELEASE_URL    "https://api.github.com/repos/v2fly/geoip/releases/latest"

static void
createOutputArray(Json::Value& outputArray, const std::vector<std::string>& usedSections) {
    // {
    //     "type": "v2rayGeoIPDat",
    //     "action": "output",
    //     "args": {
    //     "outputDir": "./output",
    //     "outputName": "geoip-only-cn-private.dat",
    //     "wantedList": ["cn", "private"]
    //     }
    // }
    Json::Value wantedList(Json::arrayValue);
    Json::Value outputObj, argsObj;

    for (const auto& section : usedSections) {
        wantedList.append(section);
    }

    argsObj["outputDir"] = "./output";
    argsObj["outputName"] = GEOIP_FILE_NAME;
    argsObj["wantedList"] = wantedList;

    outputObj["type"] = "v2rayGeoIPDat";
    outputObj["action"] = "output";
    outputObj["args"] = argsObj;

    outputArray.append(outputObj);
}

static void
addPrivateSource(Json::Value& inputArray, std::vector<std::string>& usedSections) {
    Json::Value obj;

    obj["type"] = "private";
    obj["action"] = "add";

    inputArray.append(obj);
    usedSections.push_back("private");
}

std::optional<std::string>
downloadV2ipSourceCode() {
    bool status = 1;

    uint8_t curr_attempt = 0;
    const uint8_t attempt_cnt = 5;

    try {
        downloadFile(V2IP_API_LAST_RELEASE_URL, V2IP_RELEASE_REQ_FILE_NAME);
    }  catch (std::exception& e) {
        LOG_ERROR(e.what());
        LOG_ERROR("Failed to perform API request for V2IP repository");

        return std::nullopt;
    }

    Json::Value request;
    status = readJsonFromFile(V2IP_RELEASE_REQ_FILE_NAME, request);

    if (!status) {
        LOG_ERROR("Deserialization of the repository request failed");
        return std::nullopt;
    }
    fs::remove(V2IP_RELEASE_REQ_FILE_NAME);

    std::string lastReleaseUrl = request["tarball_url"].asString();

    for(curr_attempt = 0; curr_attempt < attempt_cnt; ++curr_attempt) {
        try {
            downloadFile(lastReleaseUrl, V2IP_SRC_FILE_NAME);
        }  catch (std::exception& e) {
            LOG_ERROR(e.what());
            LOG_WARNING("Failed to download the V2IP source code, performing another attempt...");

            std::this_thread::sleep_for(std::chrono::seconds(DOWNLOAD_TRY_DELAY_SEC));

            continue;
        }

        LOG_INFO("V2IP source code was successfully downloaded");
        break;
    }

    if (curr_attempt == attempt_cnt) {
        LOG_ERROR("V2IP source code could not be downloaded after several attempts");
        return std::nullopt;
    }

    return V2IP_SRC_FILE_NAME;
}

std::optional<fs::path>
runV2ipToolchain(const std::string& rootPath) {
    const fs::path kCurrentDir = fs::current_path();
    std::optional<fs::path> outFilePath;

    fs::current_path(rootPath.c_str());

    int result = std::system("go run ./");

    if (result == 0) {
        outFilePath = fs::current_path() / "output" / GEOIP_FILE_NAME;
        LOG_INFO("IP address list building with XRay tools has been successfully completed");
    } else {
        outFilePath = std::nullopt;
        LOG_ERROR("Failed to build IP address list using XRay tools");
    }

    fs::current_path(kCurrentDir);

    return outFilePath;
}

void
addIPSource(const DownloadedSourcePair& source, Json::Value& v2ipInputArray) {
    Json::Value objRoot, objArgs;

    objArgs["name"] = source.first.section;
    objArgs["uri"] = source.second.string();

    objRoot["type"] = "text";
    objRoot["action"] = "add";
    objRoot["args"] = objArgs;

    v2ipInputArray.append(objRoot);
}

bool
saveIPSources(const std::string& v2ipRootPath, Json::Value& v2ipInputArray, std::vector<std::string>& usedSections) {
    Json::Value configObj;
    Json::Value outputArray(Json::arrayValue);
    fs::path configPath;
    bool status;

    configPath = v2ipRootPath;
    configPath /= "config.json";

    // Add dummy private section
    addPrivateSource(v2ipInputArray, usedSections);

    createOutputArray(outputArray, usedSections);

    configObj["input"] = v2ipInputArray;
    configObj["output"] = outputArray;

    status = writeJsonToFile(configPath.string(), configObj);

    return status;
}
