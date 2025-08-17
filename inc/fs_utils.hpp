#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <filesystem>

namespace fs = std::filesystem;

size_t countLinesInFile(const fs::path& filePath);

size_t removeDuplicateLines(const std::string& fileAPath, const std::string& fileBPath);

void joinTwoFiles(const std::string& fileAPath, const std::string& fileBPath);

#endif // FS_UTILS_H
