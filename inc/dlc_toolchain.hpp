#ifndef DLC_TOOLCHAIN_HPP
#define DLC_TOOLCHAIN_HPP

#include <optional>

#include "fs_utils.hpp"

extern const fs::path gkDlcToolchainDir;

std::optional<std::string> downloadDlcSourceCode();

bool clearDlcDataSection(std::string dlcRootPath);

bool addDomainSource(const std::string& dlcRootPath, const fs::path& sourceFilePath, const std::string& section);

std::optional<fs::path> runDlcToolchain(const std::string& rootPath);

#endif // DLC_TOOLCHAIN_HPP
