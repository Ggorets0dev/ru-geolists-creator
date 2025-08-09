#include "ruadlist.hpp"

#define MAX_DOMAIN_SIZE         56
#define DOMAIN_REGEX            R"((?:[a-zA-Z0-9-]+\.)+(?:by|ru|ua|info|sex|su|life|com|ml|best|net|site|biz|xyz))"

bool
extractDomainsFromFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    std::ifstream inputFile(inputFilePath);

    if (!inputFile.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + inputFilePath);
        return false;
    }

    std::ofstream outputFile(outputFilePath);

    if (!outputFile.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + outputFilePath);
        return false;
    }

    std::string currentDomain;
    std::string lastFoundDomain = "";

    currentDomain.reserve(MAX_DOMAIN_SIZE);
    lastFoundDomain.reserve(MAX_DOMAIN_SIZE);

    std::string line;
    std::regex domainRegex(DOMAIN_REGEX);
    std::smatch match;

    while (std::getline(inputFile, line)) {
        auto begin = std::sregex_iterator(line.begin(), line.end(), domainRegex);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it) {
            match = *it;

            currentDomain = match.str();

            if (currentDomain == lastFoundDomain || checkKeywordWhitelist(currentDomain)) {
                continue;
            }

            outputFile << currentDomain << std::endl;
            lastFoundDomain = std::move(currentDomain);
        }
    }

    inputFile.close();
    outputFile.close();

    return true;
}

bool
parseRuadlistUpdateDatetime(const Json::Value &value, std::time_t& dtOut) {
    std::tm tm = {};
    auto dateObj = value["commit"]["commit"]["committer"]["date"];

    std::istringstream ss(dateObj.asString());

    // Parse date and time with ISO 8601 (example 2024-12-20T14:11:25Z)
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

    if (ss.fail()) {
        LOG_ERROR("Failed to parse last change datetime from RuAdList");
        return false;
    }

    // Convert to time_t (UNIX-time)
    return std::mktime(&tm);
}
