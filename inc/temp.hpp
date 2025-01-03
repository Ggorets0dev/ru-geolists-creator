#ifndef TEMP_HPP
#define TEMP_HPP

#include <filesystem>

namespace fs = std::filesystem;

#define TEMP_DIR_NAME		"./temp"

#define CREATE_TEMP_DIR() \
    if (!fs::exists(TEMP_DIR_NAME)) { \
		fs::create_directory(TEMP_DIR_NAME); \
	}

#define ENTER_TEMP_DIR()    fs::current_path(TEMP_DIR_NAME)

#define EXIT_TEMP_DIR()     fs::current_path("./..")

#endif /* TEMP_HPP */
