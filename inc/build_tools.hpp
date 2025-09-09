#ifndef BUILD_TOOLS_HPP
#define BUILD_TOOLS_HPP

#include "config.hpp"

struct GeoListsPaths {
    fs::path listDomain;
    fs::path listIP;
};

void createReleaseNotes(const GeoListsPaths& paths, const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources);

#endif // BUILD_TOOLS_HPP
