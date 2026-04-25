#include <forward_list>
#include <fstream>
#include <unordered_set>

#include "fs_utils_temp.hpp"
#include "log.hpp"
#include "fs_utils.hpp"

#include <algorithm>

using namespace FS::Utils::Temp;

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

bool isDirEmpty(const fs::path& p, const bool allowNonExist) {
    if (!fs::exists(p) || !fs::is_directory(p)) {
        if (allowNonExist)  return true;
        throw std::runtime_error("Path does not exist or is not a directory");
    }

    return fs::directory_iterator(p) == fs::directory_iterator();
}

size_t removeDuplicateLines(const std::string& inputPath, const std::string* outputPath) {
    std::vector<std::string> lines;
    std::string line;

    std::ifstream in(inputPath);
    if (!in.is_open()) throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + inputPath);

    while (std::getline(in, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    in.close();

    size_t removedCount = lines.size();

    std::sort(lines.begin(), lines.end());
    lines.erase(std::unique(lines.begin(), lines.end()), lines.end());

    std::string targetPath = (outputPath == nullptr) ? inputPath + ".tmp" : *outputPath;

    removedCount -= lines.size();

    std::ofstream out(targetPath);
    for (const auto& l : lines) {
        out << l << "\n";
    }
    out.close();

    if (outputPath == nullptr) {
        fs::rename(targetPath, inputPath);
    }

    return removedCount;
}

size_t removeDuplicateLines(const std::string& fileAPath, const std::string& fileBPath) {
    // Открываем файл B и читаем строки в множество
    std::unordered_set<std::string> linesInB;
    const std::string* fileForReplace;
    const std::string* fileForSearch;
    size_t lineCntA, lineCntB, dupeCnt;
    SessionTempFileRegistry tfr("remove_duplicates");

    const auto tempFile = tfr.createTempFile("txt");

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

    std::ofstream tempFileReplace(tempFile.lock()->path);
    if (!tempFileReplace.is_open()) {
        throw std::ios_base::failure(FILE_OPEN_ERROR_MSG + std::string(tempFile.lock()->path));
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

    fs::remove(*fileForReplace);
    fs::rename(tempFile.lock()->path, *fileForReplace);

    return dupeCnt;
}

void joinTwoFiles(const std::string& fileAPath, const std::string& fileBPath) {
    std::ifstream fileB(fileBPath, std::ios::binary);
    if (!fileB.is_open()) {
        throw std::ios_base::failure("Could not open file B: " + fileBPath);
    }

    std::fstream fileA(fileAPath, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
    if (!fileA.is_open()) {
        throw std::ios_base::failure("Could not open file A: " + fileAPath);
    }

    fileA.seekg(0, std::ios::end);
    std::streampos fileSize = fileA.tellg();

    if (fileSize > 0) {
        fileA.seekg(-1, std::ios::cur);
        char lastChar;
        fileA.read(&lastChar, 1);

        if (lastChar != '\n') {
            fileA.clear();
            fileA.seekp(0, std::ios::end);
            fileA.put('\n');
        }
    }

    fileA.clear();
    fileA.seekp(0, std::ios::end);
    fileA << fileB.rdbuf();

    fileA.close();
    fileB.close();
}

fs::path addPathPostfix(const fs::path& p, const std::string& postfix) {
    const auto filenameWithExt = p.filename().string();
    const auto pos = filenameWithExt.find('.');

    if (pos == std::string::npos) {
        return p;
    }

    const std::string filename = filenameWithExt.substr(0, pos);
    const std::string extension = filenameWithExt.substr(pos + 1);

    fs::path result = p.parent_path().string() + "/" + filename + "_" + postfix + "." + extension;

    return result;
}


bool createEmptyFile(const fs::path& path) {
    std::ofstream ofs(path, std::ios::out | std::ios::trunc);
    return ofs.good();
}