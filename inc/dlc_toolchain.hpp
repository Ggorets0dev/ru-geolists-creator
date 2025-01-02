#ifndef DLC_TOOLCHAIN_HPP
#define DLC_TOOLCHAIN_HPP

#include <thread>
#include <chrono>
#include <filesystem>
#include <optional>

#include "json_io.hpp"
#include "network.hpp"

#define DLC_SRC_FILE_NAME           "dlc_src.tar.gz"
#define DLC_RELEASE_REQ_FILE_NAME   "dlc_req.json"

extern std::optional<std::string>
downloadDlcSourceCode();

extern bool
clearDlcDataSection(std::string dlcRootPath);

#endif // DLC_TOOLCHAIN_HPP
