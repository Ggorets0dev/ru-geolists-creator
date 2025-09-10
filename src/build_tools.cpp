#include "build_tools.hpp"
#include "time_tools.hpp"
#include "cli_args.hpp"
#include "ipc_chain.hpp"
#include "log.hpp"

// ============== PROTOBUF headers
#include "release_notes.pb.h"
// ==============

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define RELEASE_NOTES_TXT_FILENAME  "release_notes.txt"

static void saveToText(const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources) {
    fs::path notesPath = fs::current_path() / OUTPUT_FOLDER_NAME / RELEASE_NOTES_TXT_FILENAME;
    std::ofstream releaseNotes(notesPath);

    releaseNotes << "Added IP/Domain sources:" << std::endl;
    printDownloadedSources(releaseNotes, downloadedSources);

    releaseNotes << "\n\n";

    releaseNotes << "ReFilter lists datetime: " << parseUnixTime(config.refilterTime) << std::endl;
    releaseNotes << "V2Ray lists datetime: " << parseUnixTime(config.v2rayTime) << std::endl;
    releaseNotes << "RuAdList datetime: " << parseUnixTime(config.ruadlistTime) << std::endl;

    releaseNotes << "\n\n";

    releaseNotes << "Build datetime: " << parseUnixTime(std::time(nullptr));

    releaseNotes.close();
}

static void saveToFIFO(const GeoListsPaths& paths, const std::vector<DownloadedSourcePair>& downloadedSources) {
    geo_release::ReleaseNotes releaseNotes;
    geo_release::ReleaseNotes::FilesPaths filesPaths;
    fs::path pathFIFO {RGC_RELEASE_NOTES_FIFO_PATH};
    int err, fd;

    filesPaths.set_domain_list(paths.listDomain);
    filesPaths.set_ip_list(paths.listIP);

    releaseNotes.set_time(parseUnixTime(std::time(nullptr)));
    releaseNotes.set_allocated_files_paths(&filesPaths);

    for (const auto& dl_source : downloadedSources) {
        auto pb_source = releaseNotes.add_sources();

        pb_source->set_section(dl_source.first.section);
        pb_source->set_type((geo_release::ReleaseNotes::SourceType)dl_source.first.type);
    }

    // Create all DIRs needed for FIFO
    fs::create_directories(pathFIFO.parent_path());

    if (fs::exists(pathFIFO)) {
        fs::remove(pathFIFO);
    }

    // SECTION: Creating and opening FIFO for IPC
    err = mkfifo(RGC_RELEASE_NOTES_FIFO_PATH, RGC_RELEASE_NOTES_FIFO_PERMS);

    if (err == -1) {
        throw std::runtime_error("Failed to create FIFO for IPC");
    }

    // Sending signal to parrent for starting IPC
    kill(getppid(), SIGUSR1);

    fd = open(RGC_RELEASE_NOTES_FIFO_PATH, O_WRONLY);

    if (fd == -1) {
        throw std::runtime_error("Failed to open FIFO for IPC");
    }

    err = releaseNotes.SerializeToFileDescriptor(fd);

    if (err == 0) {
        close(fd);
        throw std::runtime_error("Failed to write in FIFO for IPC");
    }

    err = close(fd);

    if (err == -1) {
        throw std::runtime_error("Failed to close FIFO for IPC");
    }
    // !SECTION

    LOG_INFO("Release notes were sent to parent by IPC successfully");
}

void createReleaseNotes(const GeoListsPaths& paths,
                        const RgcConfig& config,
                        const std::vector<DownloadedSourcePair>& downloadedSources) {

    // Form TXT release notes for user
    saveToText(config, downloadedSources);

    if (gCmdArgs.isChild) {
        try {
            // Form protobuf release notes for IPC
            saveToFIFO(paths, downloadedSources);
        } catch (const std::runtime_error& e) {
            LOG_ERROR(e.what());
            LOG_ERROR("Failed to perform IPC in current build");
        }
    }
}
