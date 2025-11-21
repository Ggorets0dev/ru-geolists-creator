#include "fs_utils.hpp"
#include "log.hpp"

#include <fstream>
#include <unordered_set>

#define TEMP_FILE_NAME "filter_temp.txt"

void removePath(const std::string& path) {
    if (!path.empty() && fs::exists(path))
        fs::remove_all(path);
}

size_t countLinesInFile(const fs::path& filePath) {
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        throw std::ios_base::failure(FILE_LOCATE_ERROR_MSG + filePath.string());
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + filePath.string());
    }

    size_t line_count = 0;
    std::string line;
    while (std::getline(file, line)) {
        ++line_count;
    }

    return line_count;
}

size_t removeDuplicateLines(const std::string& fileAPath, const std::string& fileBPath) {
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

void joinTwoFiles(const std::string& fileAPath, const std::string& fileBPath) {
    std::ifstream fileB(fileBPath, std::ios::binary);

    if (!fileB.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + fileBPath);
    }

    std::ofstream fileA(fileAPath, std::ios::binary | std::ios::app);

    if (!fileA.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + fileAPath);
    }

//    fileA.seekp(0, std::ios::end);
//    if (fileA.tellp() != 0) {
//        fileA << '\n';
//    }

    fileA << fileB.rdbuf();

    fileA.close();
    fileB.close();
}

fs::path addPathPostfix(const fs::path path, const std::string& postfix) {
    return path.parent_path() /
                (path.stem().string() + "_" + postfix +
                 path.extension().string());
}
