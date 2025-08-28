#ifndef BUILD_TOOLS_HPP
#define BUILD_TOOLS_HPP

#include "config.hpp"

struct GeoListsPaths {
    fs::path domain_list;
    fs::path ip_list;
};

void createReleaseNotes(const GeoListsPaths& paths, const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources);

#endif // BUILD_TOOLS_HPP
