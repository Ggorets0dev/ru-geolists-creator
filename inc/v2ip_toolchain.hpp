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

namespace fs = std::filesystem;

extern std::optional<std::string>
downloadV2ipSourceCode();

extern std::optional<fs::path>
runV2ipToolchain(const std::string& rootPath);

extern void
addIPSource(const DownloadedSourcePair& source, Json::Value& v2ipInputArray);

extern bool
saveIPSources(const std::string& v2ipRootPath, Json::Value& v2ipInputArray, std::vector<std::string>& usedSections);

#endif // V2IP_TOOLCHAIN_HPP
