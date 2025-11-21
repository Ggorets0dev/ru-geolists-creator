#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <filesystem>

namespace fs = std::filesystem;

size_t countLinesInFile(const fs::path& filePath);

size_t removeDuplicateLines(const std::string& fileAPath, const std::string& fileBPath);

void joinTwoFiles(const std::string& fileAPath, const std::string& fileBPath);

fs::path addPathPostfix(fs::path path, const std::string& postfix);

void removePath(const std::string& path);

#endif // FS_UTILS_H
