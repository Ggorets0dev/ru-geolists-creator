#ifndef BUILD_TOOLS_HPP
#define BUILD_TOOLS_HPP

#include "config.hpp"

#define GEOSITE_FILE_NAME       "geosite.dat"
#define GEOIP_FILE_NAME         "geoip.dat"
#define RELEASE_NOTES_FILENAME  "release_notes.txt"

struct GeoListsPaths {
    fs::path listDomain;
    fs::path listIP;
    fs::path releaseNotes;
};

void createReleaseNotes(const GeoListsPaths& paths, const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources);

#endif // BUILD_TOOLS_HPP
