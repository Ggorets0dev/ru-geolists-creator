#ifndef V2IP_TOOLCHAIN_HPP
#define V2IP_TOOLCHAIN_HPP

#include <thread>
#include <chrono>
#include <filesystem>
#include <optional>
#include <unistd.h>

#include "config.hpp"
#include "network.hpp"
#include "log.hpp"
#include "json_io.hpp"

#define V2IP_SRC_FILE_NAME           "v2ip_src.tar.gz"
#define V2IP_RELEASE_REQ_FILE_NAME   "v2ip_req.json"

extern std::optional<std::string>
downloadV2ipSourceCode();

extern bool
runToolchain(const std::string& rootPath);

#endif // V2IP_TOOLCHAIN_HPP
