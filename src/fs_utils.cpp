#include "fs_utils.hpp"
#include "log.hpp"

#include <fstream>
#include <unordered_set>

#define TEMP_FILE_NAME "filter_temp.txt"

size_t countLinesInFile(const fs::path& file_path) {
    if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
        throw std::ios_base::failure(FILE_LOCATE_ERROR_MSG + file_path.string());
    }

    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + file_path.string());
    }

    size_t line_count = 0;
    std::string line;
    while (std::getline(file, line)) {
        ++line_count;
    }

    return line_count;
}

size_t
removeDuplicateLines(const std::string& fileAPath, const std::string& fileBPath) {
    // Открываем файл B и читаем строки в множество
    std::unordered_set<std::string> linesInB;
    const std::string* fileForReplace;
    const std::string* fileForSearch;
    size_t lineCntA, lineCntB, dupeCnt;

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


    dupeCnt = 0;

    while (std::getline(fileReplace, line)) {
        if (linesInB.find(line) == linesInB.end()) {
            tempFileReplace << line << std::endl;
        } else {
            ++dupeCnt;
        }
    }

    tempFileReplace.close();
    fileReplace.close();

    fs::remove(fileAPath);
    fs::rename(TEMP_FILE_NAME, fileAPath);

    return dupeCnt;
}
