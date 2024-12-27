#include "filter.hpp"

#define TEMP_FILE_NAME "filer_temp.txt"

const std::vector<std::string> kKeywordWhitelist = {
    "google",
    "yandex",
    "vk.com",
    "ya.ru",
    "ok.ru",
    "yastatic.net",
    "alfabank.ru"
};

bool
checkKeywordBlacklist(std::string_view domain) {
    for (uint8_t i(0); i < kKeywordWhitelist.size(); ++i) {
        if (domain.find(kKeywordWhitelist[i]) != std::string::npos) {
            return true;
        }
    }

    return false;
}

bool
removeDuplicateDomains(const std::string& fileAPath, const std::string& fileBPath) {
    // Открываем файл B и читаем строки в множество
    std::unordered_set<std::string> linesInB;

    std::ifstream fileB(fileBPath);
    if (!fileB.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + fileBPath);
        return false;
    }

    std::string line;
    while (std::getline(fileB, line)) {
        linesInB.insert(line);
    }
    fileB.close();

    // Открываем файл A и фильтруем строки
    std::ifstream fileA(fileAPath);
    if (!fileA.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + fileAPath);
        return false;
    }

    std::ofstream tempFileA(TEMP_FILE_NAME);
    if (!tempFileA.is_open()) {
        std::string logMsg = FILE_OPEN_ERROR_MSG;
        logMsg += TEMP_FILE_NAME;

        LOG_ERROR(logMsg);
        return false;
    }

    while (std::getline(fileA, line)) {
        if (linesInB.find(line) == linesInB.end()) {
            tempFileA << line << "\n";
        }
    }

    tempFileA.close();
    fileA.close();

    fs::remove(fileAPath);
    fs::rename(TEMP_FILE_NAME, fileAPath);

    return true;
}
