#ifndef DLC_TOOLCHAIN_HPP
#define DLC_TOOLCHAIN_HPP

#include <optional>

#include "fs_utils.hpp"

#define DLC_SRC_FILE_NAME           "dlc_src.tar.gz"
#define DLC_RELEASE_REQ_FILE_NAME   "dlc_req.json"

extern std::optional<std::string>
downloadDlcSourceCode();

extern bool
clearDlcDataSection(std::string dlcRootPath);

extern bool
addDomainSource(const std::string& dlcRootPath, const fs::path& sourceFilePath, const std::string& section);

extern std::optional<fs::path>
runDlcToolchain(const std::string& rootPath);

#endif // DLC_TOOLCHAIN_HPP
