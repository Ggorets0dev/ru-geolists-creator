#ifndef BUILD_TOOLS_HPP
#define BUILD_TOOLS_HPP

#include <utility>

#include "config.hpp"

#define GEOSITE_BASE_FILENAME        "geosite"
#define GEOIP_BASE_FILENAME          "geoip"

#define V2RAY_FILES_EXT              ".dat"
#define SING_FILES_EXT               ".db"

#define RELEASE_NOTES_FILENAME      "release_notes.txt"

struct GeoReleasePack {
    GeoReleasePack(fs::path  listDomain, fs::path  listIP) :
        listDomain(std::move(listDomain)), listIP(std::move(listIP)) {}

    fs::path listDomain;
    fs::path listIP;
};

struct GeoReleases {
    std::vector<GeoReleasePack> packs;
    fs::path releaseNotes;
    bool isEmpty;
};

void createReleaseNotes(const GeoReleases& paths, const std::vector<DownloadedSourcePair>& downloadedSources);

#endif // BUILD_TOOLS_HPP
