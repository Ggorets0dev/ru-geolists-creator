#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <filesystem>

namespace fs = std::filesystem;

size_t countLinesInFile(const fs::path& file_path);

#endif // FS_UTILS_H
