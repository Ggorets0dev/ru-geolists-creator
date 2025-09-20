#ifndef BUILD_TOOLS_HPP
#define BUILD_TOOLS_HPP

#include "config.hpp"

#define GEOSITE_FILENAME_DAT       "geosite.dat"
#define GEOIP_FILENAME_DAT         "geoip.dat"

#define GEOSITE_FILENAME_DB       "geosite.db"
#define GEOIP_FILENAME_DB         "geoip.db"

#define RELEASE_NOTES_FILENAME  "release_notes.txt"

struct GeoReleasePack {
    GeoReleasePack(const fs::path& listDomain, const fs::path& listIP) :
        listDomain(listDomain), listIP(listIP) {}

    fs::path listDomain;
    fs::path listIP;
};

struct GeoReleases {
    std::vector<GeoReleasePack> packs;
    fs::path releaseNotes;
};

void createReleaseNotes(const GeoReleases& paths, const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources);

#endif // BUILD_TOOLS_HPP
