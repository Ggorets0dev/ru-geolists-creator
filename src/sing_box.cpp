#include <algorithm>
#include <fstream>
#include <set>
#include <json/json.h>

#include "config.hpp"
#include "main_sources.hpp"

bool generateSingBoxRuleSet(
    const std::vector<DownloadedSourcePair>& sources,
    const fs::path& savePath
) {
    try {
        auto config = getCachedConfig();

        Json::Value root;
        root["version"] = 1;

        std::set<std::string> fullDomains;
        std::set<std::string> suffixDomains;
        std::set<std::string> uniqueIps;

        for (const auto&[fst, snd] : sources) {
            const auto& source = config->sources.at(fst);
            std::ifstream file(snd);
            if (!file.is_open()) continue;

            std::string line;
            while (std::getline(file, line)) {
                line.erase(0, line.find_first_not_of(" \t\r\n"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);

                if (line.empty() || line[0] == '#') continue;

                if (source.inetType == Source::InetType::DOMAIN) {
                    size_t dotCount = std::count(line.begin(), line.end(), '.');

                    if (dotCount == 1) {
                        suffixDomains.insert("." + line);
                    } else if (dotCount > 1) {
                        fullDomains.insert(line);
                    }
                } else {
                    uniqueIps.insert(line);
                }
            }
        }

        Json::Value ruleObj;

        if (!fullDomains.empty()) {
            Json::Value domainArray(Json::arrayValue);
            for (const auto& d : fullDomains) domainArray.append(d);
            ruleObj["domain"] = domainArray;
        }

        if (!suffixDomains.empty()) {
            Json::Value suffixArray(Json::arrayValue);
            for (const auto& d : suffixDomains) suffixArray.append(d);
            ruleObj["domain_suffix"] = suffixArray;
        }

        if (!uniqueIps.empty()) {
            Json::Value ipArray(Json::arrayValue);
            for (const auto& ip : uniqueIps) ipArray.append(ip);
            ruleObj["ip_cidr"] = ipArray;
        }

        if (fullDomains.empty() && suffixDomains.empty() && uniqueIps.empty())
            return false;

        Json::Value rulesArray(Json::arrayValue);
        rulesArray.append(ruleObj);
        root["rules"] = rulesArray;

        std::ofstream outFile(savePath);
        if (!outFile.is_open()) return false;

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &outFile);

        return true;
    } catch (...) {
        return false;
    }
}

bool compileSingBoxRuleSet(
    const fs::path& binaryPath,
    const fs::path& jsonPath,
    const fs::path& srsPath
) {
    if (!fs::exists(binaryPath)) {
        return false;
    }

    if (!fs::exists(jsonPath)) {
        return false;
    }

    std::string command = binaryPath.string() + " rule-set compile " +
                          jsonPath.string() + " -o " +
                          srsPath.string();

    int result = std::system(command.c_str());

    return result == 0;
}