#include "fs_utils.hpp"
#include "log.hpp"

#include <fstream>

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
