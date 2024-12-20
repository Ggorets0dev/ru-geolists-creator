#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include <fstream>
#include <string>
#include <optional>
#include <archive.h>
#include <archive_entry.h>

#include "log.hpp"

extern std::optional<std::string>
extractTarGz(const std::string& archivePath, const std::string& destDirPath);

#endif // ARCHIVE_HPP
