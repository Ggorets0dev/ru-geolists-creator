#ifndef GEO_MANAGER_HPP
#define GEO_MANAGER_HPP

#include <optional>

#include "json_io.hpp"
#include "main_sources.hpp"

extern const fs::path gkGeoManagerDir;

std::optional<std::string> setupGeoManagerBinary();

bool convertGeolist(const std::string& binPath, Source::InetType type,
                    const std::string& inFormat, const std::string& outFormat,
                    const std::string& inPath, const std::string& outPath);

#endif // GEO_MANAGER_HPP
