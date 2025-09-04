#ifndef TEMP_HPP
#define TEMP_HPP

#include <filesystem>

namespace fs = std::filesystem;

#define TEMP_DIR_NAME            getTempDirPath()

#define CREATE_TEMP_DIR() \
if (!fs::exists(TEMP_DIR_NAME)) { \
        fs::create_directory(TEMP_DIR_NAME); \
}

#define CLEAR_TEMP_DIR() \
if (fs::exists(TEMP_DIR_NAME) && fs::is_directory(TEMP_DIR_NAME)) { \
        for (const auto& entry : fs::directory_iterator(TEMP_DIR_NAME)) { \
            fs::remove_all(entry.path()); \
    } \
}

#define ENTER_TEMP_DIR()        fs::current_path(TEMP_DIR_NAME)
#define EXIT_TEMP_DIR()         fs::current_path(fs::current_path().parent_path())

inline fs::path getSystemTempDir() {
    const char* tmpDir = std::getenv("TMPDIR");

    return tmpDir ? fs::path(tmpDir) : fs::path("/tmp");
}

inline fs::path& getTempDirPath() {
    static fs::path tempDir = getSystemTempDir() / ("ru-geolists-creator_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));

    return tempDir;
}

#endif /* TEMP_HPP */
