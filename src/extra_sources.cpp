#include "extra_sources.hpp"

ExtraSource::ExtraSource(const Json::Value& value) {
    this->type = (value["type"].asString() == "domain") ? ExtraSource::Type::DOMAIN : ExtraSource::Type::IP;
	this->url = value["url"].asString();
	this->section = value["section"].asString();

    std::string operation = value["operation"].asString();

    if (operation == "block") {
        this->operation = ExtraSource::Operation::BLOCK;
    } else if (operation == "direct") {
        this->operation = ExtraSource::Operation::DIRECT;
    } else { // proxy
        this->operation = ExtraSource::Operation::PROXY;
    }
}
