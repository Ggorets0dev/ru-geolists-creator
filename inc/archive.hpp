#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include <string>
#include <optional>

extern std::optional<std::string>
extractTarGz(const std::string& archivePath, const std::string& destDirPath);

#endif // ARCHIVE_HPP
