#include "main_sources.hpp"

#define DOWNLOADED_SOURCES_DELIMETER "---------"

Source::Source(Source::Type type, const std::string& section) {
    this->type = type;
    this->section = section;
}

void Source::print(std::ostream& stream) const {
    stream << "Type: " << sourceTypeToString(this->type) << std::endl;
    stream << "Section: " << this->section << std::endl;
}

std::string
sourceTypeToString(Source::Type type) {
    switch(type) {
    case Source::Type::IP:
        return "ip";
    case Source::Type::DOMAIN:
        return "domain";
    }
}

Source::Type
sourceStringToType(std::string_view str) {
    if (str == "ip") {
        return Source::Type::IP;
    } else { //  domain
        return Source::Type::DOMAIN;
    }
}

void
printDownloadedSources(std::ostream& stream, const std::vector<DownloadedSourcePair>& downloadedSources) {
    const char delimeter[] = DOWNLOADED_SOURCES_DELIMETER;

    stream << delimeter << std::endl;

    for (const auto& source : downloadedSources) {
        source.first.print(stream);
        stream << "Path: " << source.second << std::endl;
        stream << delimeter << std::endl;
    }
}
