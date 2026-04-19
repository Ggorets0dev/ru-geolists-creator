#include <algorithm>
#include <fstream>
#include <set>
#include <fmt/format.h>
#include <json/json.h>

#include "build_tools.hpp"
#include "fs_utils_temp.hpp"
#include "log.hpp"
#include "main_sources.hpp"

std::optional<std::vector<fs::path>> generateSingBoxRuleSets(
    const std::vector<DownloadedSourcePair>& sources,
    const SourcesStorage& storage
) {
    std::vector<fs::path> rulesetsPaths;

    struct SectionData {
        std::set<std::string> fullDomains;
        std::set<std::string> suffixDomains;
        std::set<std::string> uniqueIps;
    };

    try {
        std::map<std::string, SectionData> groupedData;

        // ===================
        // FILL ARRAYS WITH SUBNETS AND DOMAINS
        // ===================
        for (const auto& [sourceId, filePath] : sources) {
            const auto& source = storage.at(sourceId);
            auto& data = groupedData[source.section];

            std::ifstream file(filePath);
            if (!file.is_open()) continue;

            std::string line;
            while (std::getline(file, line)) {
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);

                if (line.empty() || line[0] == '#') continue;

                if (source.inetType == Source::InetType::DOMAIN) {
                    data.fullDomains.insert(line);

                    if (std::count(line.begin(), line.end(), '.') == 1) {
                        data.suffixDomains.insert("." + line);
                    }
                } else { // IP CIDR
                    data.uniqueIps.insert(line);
                }
            }
        }
        // ===================

        // ===================
        // CREATE JSON RULESET FILE WITH ARRAYS
        // ===================
        for (const auto& [sectionName, data] : groupedData) {
            if (data.fullDomains.empty() && data.suffixDomains.empty() && data.uniqueIps.empty())
                continue;

            const FS::Utils::Temp::SessionTempFileRegistry registry(sectionName);
            const fs::path ruleSetJsonPath = registry.createTempFileDetached("json")->path;

            Json::Value root;
            root["version"] = 1;

            Json::Value ruleObj;
            if (!data.fullDomains.empty()) {
                Json::Value domainArray(Json::arrayValue);
                for (const auto& d : data.fullDomains) domainArray.append(d);
                ruleObj["domain"] = domainArray;
            }

            if (!data.suffixDomains.empty()) {
                Json::Value suffixArray(Json::arrayValue);
                for (const auto& d : data.suffixDomains) suffixArray.append(d);
                ruleObj["domain_suffix"] = suffixArray;
            }

            if (!data.uniqueIps.empty()) {
                Json::Value ipArray(Json::arrayValue);
                for (const auto& ip : data.uniqueIps) ipArray.append(ip);
                ruleObj["ip_cidr"] = ipArray;
            }

            Json::Value rulesArray(Json::arrayValue);
            rulesArray.append(ruleObj);
            root["rules"] = rulesArray;

            std::ofstream outFile(ruleSetJsonPath);
            if (!outFile.is_open()) continue;

            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
            writer->write(root, &outFile);

            rulesetsPaths.push_back(ruleSetJsonPath);
        }
        // ===================

    } catch (...) {
        return std::nullopt;
    }

    return rulesetsPaths;
}

std::optional<std::vector<fs::path>> compileSingBoxRuleSets(
    const fs::path& binaryPath,
    const fs::path& targetDir,
    const std::vector<fs::path>& jsonPaths
) {

    if (!fs::exists(binaryPath) || jsonPaths.empty()) {
        return std::nullopt;
    }

    std::vector<fs::path> rulesetsPaths;
    rulesetsPaths.reserve(jsonPaths.size());

    for (const auto& path : jsonPaths) {
        const std::string basePrefix = FS::Utils::Temp::SessionTempFileRegistry::getBasePrefix(path);
        const std::string ruleSetSrsFilename = fmt::format("{}-{}.{}",
            RULESET_BASE_FILENAME,
            basePrefix,
            SING_RS_FILES_EXT);

        const fs::path ruleSetSrsPath = targetDir / ruleSetSrsFilename;
        const std::string command = binaryPath.string() + " rule-set compile " +
                         path.string() + " -o " +
                         ruleSetSrsPath.string();

        if (int result = std::system(command.c_str()); result == 0) {
            LOG_INFO("Compiled ruleset from json to srs for base prefix: {}", basePrefix);
        } else {
            LOG_WARNING("Failed to compile json to srs for base prefix: {}", basePrefix);
        }

        rulesetsPaths.push_back(ruleSetSrsPath);
    }

    if (!rulesetsPaths.empty()) {
        return rulesetsPaths;
    }

    return std::nullopt;
}