#include "main_sources.hpp"

Source::Source(Source::Type type, const std::string& section) {
    this->type = type;
    this->section = section;
}

void Source::print() const {
    std::cout << "Type: " << sourceTypeToString(this->type) << std::endl;
    std::cout << "Section: " << this->section << std::endl;
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
