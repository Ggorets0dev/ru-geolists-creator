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
checkKeywordWhitelist(std::string_view domain) {
    for (uint8_t i(0); i < kKeywordWhitelist.size(); ++i) {
        if (domain.find(kKeywordWhitelist[i]) != std::string::npos) {
            return true;
        }
    }

    return false;
}

void
removeDuplicateDomains(const std::string& fileAPath, const std::string& fileBPath) {
    // Открываем файл B и читаем строки в множество
    std::unordered_set<std::string> linesInB;
    const std::string* fileForReplace;
    const std::string* fileForSearch;
    size_t lineCntA, lineCntB;

    // Ищем файл с меньшим количеством строк, его выгружаем в ОЗУ для поиска

    lineCntA = countLinesInFile(fileAPath);
    lineCntB = countLinesInFile(fileBPath);

    if (lineCntA < lineCntB) {
        fileForReplace = &fileBPath;
        fileForSearch = &fileAPath;
    } else {
        fileForReplace = &fileAPath;
        fileForSearch = &fileBPath;
    }

    std::ifstream fileSearch(*fileForSearch);
    if (!fileSearch.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + *fileForSearch);
    }

    std::string line;
    while (std::getline(fileSearch, line)) {
        linesInB.insert(line);
    }
    fileSearch.close();

    // Открываем файл A и фильтруем строки
    std::ifstream fileReplace(*fileForReplace);
    if (!fileReplace.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + *fileForReplace);
    }

    std::ofstream tempFileReplace(TEMP_FILE_NAME);
    if (!tempFileReplace.is_open()) {
        std::string logMsg = FILE_OPEN_ERROR_MSG;
        logMsg += TEMP_FILE_NAME;

        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + std::string(TEMP_FILE_NAME));
    }

    while (std::getline(fileReplace, line)) {
        if (linesInB.find(line) == linesInB.end()) {
            tempFileReplace << line << std::endl;
        }
    }

    tempFileReplace.close();
    fileReplace.close();

    fs::remove(fileAPath);
    fs::rename(TEMP_FILE_NAME, fileAPath);
}
