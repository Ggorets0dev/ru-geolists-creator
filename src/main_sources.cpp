#include "main_sources.hpp"

#include <algorithm>

#define DOWNLOADED_SOURCES_DELIMETER    "------------------------------------"

#define SOURCE_FILENAME_SALT_SIZE       6

Source::Source(Source::Type type, const std::string& section) {
    this->type = type;
    this->section = section;
}

void Source::print(std::ostream& stream) const {
    stream << "Type: " << sourceTypeToString(this->type) << std::endl;
    stream << "Section: " << this->section << std::endl;
}

std::string sourceTypeToString(Source::Type type) {
    switch(type) {
    case Source::Type::IP:
        return "ip";
    case Source::Type::DOMAIN:
        return "domain";
    default:
        return "NONE";
    }
}

Source::Type sourceStringToType(std::string_view str) {
    if (str == "ip") {
        return Source::Type::IP;
    } else { //  domain
        return Source::Type::DOMAIN;
    }
}

void  printDownloadedSources(std::ostream& stream, const std::vector<DownloadedSourcePair>& downloadedSources) {
    const char delimeter[] = DOWNLOADED_SOURCES_DELIMETER;

    stream << delimeter << std::endl;

    for (const auto& source : downloadedSources) {
        source.first.print(stream);
        stream << "Path: " << source.second << std::endl;
        stream << delimeter << std::endl;
    }
}

void joinSimilarSources(std::vector<DownloadedSourcePair>& sources) {
    std::vector<bool> removeMarkers(sources.size());

    for (size_t i(0); i < sources.size(); ++i) {
        DownloadedSourcePair& parentSource = sources[i];

        if (removeMarkers[i]) {
            // Source is already joined
            continue;
        }

        for (size_t j(i + 1); j < sources.size(); ++j) {
            DownloadedSourcePair& childSource = sources[j];

            if (parentSource.first.section != childSource.first.section || parentSource.first.type != childSource.first.type || removeMarkers[j]) {
                // Sources cant be joined
                continue;
            }

            joinTwoFiles(parentSource.second, childSource.second);
            removeMarkers[j] = true;
        }
    }

    size_t index(0);
    auto removeIter = std::remove_if(sources.begin(), sources.end(), [&index, &removeMarkers](const auto& pair) {
        ++index;
        return removeMarkers[index - 1];
    });

    sources.erase(removeIter, sources.end());
}

std::string genSourceFileName(const Source& source) {
    std::string result;
    result.reserve(source.section.length() + SOURCE_FILENAME_SALT_SIZE);

    result += source.section;
    result += "_";

    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const uint8_t charsCount = chars.length();

    for (uint8_t i = 0; i < SOURCE_FILENAME_SALT_SIZE; ++i) {
        result += chars[std::rand() % charsCount];
    }

    return result;
}
