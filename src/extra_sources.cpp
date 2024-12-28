#include "extra_sources.hpp"

ExtraSource::ExtraSource(const Json::Value& value) {
    this->type = extraStringToType(value["type"].asString());
	this->url = value["url"].asString();
	this->section = value["section"].asString();
    this->operation = extraStringToOperation(value["operation"].asString());
}

std::string
extraTypeToString(ExtraSource::Type type) {
    switch(type) {
    case ExtraSource::Type::IP:
        return "ip";
    case ExtraSource::Type::DOMAIN:
        return "domain";
    }
}

ExtraSource::Type
extraStringToType(std::string_view str) {
    if (str == "ip") {
        return ExtraSource::Type::IP;
    } else { //  domain
        return ExtraSource::Type::DOMAIN;
    }
}

std::string
extraOperationToString(ExtraSource::Operation type) {
    switch(type) {
    case ExtraSource::Operation::BLOCK:
        return "block";
    case ExtraSource::Operation::PROXY:
        return "proxy";
    case ExtraSource::Operation::DIRECT:
        return "direct";
    }
}

ExtraSource::Operation
extraStringToOperation(std::string_view str) {
    if (str == "block") {
        return ExtraSource::Operation::BLOCK;
    } else if (str == "direct") {
        return ExtraSource::Operation::DIRECT;
    } else { //  proxy
        return ExtraSource::Operation::PROXY;
    }
}
