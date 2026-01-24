#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include <string>
#include <optional>

std::optional<std::string> extractTarGz(const std::string& archivePath, const std::string& destDirPath);

std::optional<std::string> createZipArchive(const std::string& folderPath, const std::string& archiveName);

#endif // ARCHIVE_HPP
