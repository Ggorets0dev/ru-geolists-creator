#include "v2ip_toolchain.hpp"
#include "build_tools.hpp"
#include "log.hpp"
#include "url_handle.hpp"
#include "fs_utils_temp.hpp"

#include <unistd.h>

#define V2IP_API_LAST_RELEASE_URL    "https://api.github.com/repos/v2fly/geoip/releases/latest"

const fs::path gkV2ipToolchainDir = fs::path(std::getenv("HOME")) / ".local" / "lib";

static void createOutputArray(Json::Value& outputArray, const std::vector<std::string>& usedSections) {
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
    argsObj["outputName"] = "geoip.dat";
    argsObj["wantedList"] = wantedList;

    outputObj["type"] = "v2rayGeoIPDat";
    outputObj["action"] = "output";
    outputObj["args"] = argsObj;

    outputArray.append(outputObj);
}

static void addPrivateSource(Json::Value& inputArray, std::vector<std::string>& usedSections) {
    Json::Value obj;

    obj["type"] = "private";
    obj["action"] = "add";

    inputArray.append(obj);
    usedSections.emplace_back("private");
}

std::optional<std::string> downloadV2ipSourceCode() {
    // ========= Temp files control
    FS::Utils::Temp::SessionTempFileRegistry tempFileReg;
    const auto v2ipReqFile = tempFileReg.createTempFile("json");
    const auto v2ipSourceFile = tempFileReg.createTempFileDetached("tar.gz");
    // =========

    LOG_INFO("Starting to download V2IP source code...");

    if (!NetUtils::tryDownloadFile(V2IP_API_LAST_RELEASE_URL, v2ipReqFile.lock()->path)) {
        LOG_ERROR("Failed to perform API request for V2IP repository");
        return std::nullopt;
    }

    Json::Value request;

    if (const bool status = readJsonFromFile(v2ipReqFile.lock()->path, request); !status) {
        LOG_ERROR("Failed to read JSON from API request (V2IP)");
        return std::nullopt;
    }

    if (const std::string lastReleaseUrl = request["tarball_url"].asString(); !NetUtils::tryDownloadFile(lastReleaseUrl, v2ipSourceFile->path)) {
        LOG_ERROR("V2IP source code could not be downloaded after several attempts");
        return std::nullopt;
    }

    return v2ipSourceFile->path;
}

std::optional<fs::path> runV2ipToolchain(const std::string& rootPath) {
    const fs::path kCurrentDir = fs::current_path();
    std::optional<fs::path> outFilePath;

    fs::current_path(rootPath.c_str());

    if (const int result = std::system("go run ./"); result == 0) {
        outFilePath = fs::current_path() / "output" / "geoip.dat";
        LOG_INFO("IP address list building with XRay tools has been successfully completed");
    } else {
        outFilePath = std::nullopt;
        LOG_ERROR("Failed to build IP address list using XRay tools");
    }

    fs::current_path(kCurrentDir);

    return outFilePath;
}

void addIPSource(const DownloadedSourcePair& source, Json::Value& v2ipInputArray) {
    Json::Value objRoot, objArgs;

    const auto config = getCachedConfig();

    objArgs["name"] = config->sources.at(source.first).section;
    objArgs["uri"] = source.second.string();

    objRoot["type"] = "text";
    objRoot["action"] = "add";
    objRoot["args"] = objArgs;

    v2ipInputArray.append(objRoot);
}

bool saveIPSources(const std::string& v2ipRootPath, Json::Value& v2ipInputArray, std::vector<std::string>& usedSections) {
    Json::Value configObj;
    Json::Value outputArray(Json::arrayValue);

    const fs::path configPath = fs::path(v2ipRootPath) / "config.json";

    // Add dummy private section
    addPrivateSource(v2ipInputArray, usedSections);

    createOutputArray(outputArray, usedSections);

    configObj["input"] = v2ipInputArray;
    configObj["output"] = outputArray;

    const bool status = writeJsonToFile(configPath.string(), configObj);

    return status;
}
