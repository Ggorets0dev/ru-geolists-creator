#ifndef V2IP_TOOLCHAIN_HPP
#define V2IP_TOOLCHAIN_HPP

#include <optional>

#include "json_io.hpp"
#include "main_sources.hpp"

#define V2IP_TOOLCHAIN_DIRNAME   "v2fly-v2ip-toolchain"

extern const fs::path gkV2ipToolchainDir;

std::optional<std::string> downloadV2ipSourceCode();

std::optional<fs::path> runV2ipToolchain(const std::string& rootPath);

void addIPSource(const DownloadedSourcePair& source, Json::Value& v2ipInputArray);

bool saveIPSources(const std::string& v2ipRootPath, Json::Value& v2ipInputArray, std::vector<std::string>& usedSections);

#endif // V2IP_TOOLCHAIN_HPP
