#ifndef FS_UTILS_H
#define FS_UTILS_H

#include "fs_utils_types_base.hpp"

#include <string>

size_t countLinesInFile(const fs::path& filePath);

size_t removeDuplicateLines(const std::string& fileAPath, const std::string& fileBPath);

void joinTwoFiles(const std::string& fileAPath, const std::string& fileBPath);

fs::path addPathPostfix(const fs::path& p, const std::string& postfix);

void removePath(const std::string& path);

#endif // FS_UTILS_H
