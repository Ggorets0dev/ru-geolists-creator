#include "extra_sources.hpp"

ExtraSource::ExtraSource(const Json::Value& value) : Source(sourceStringToType(value["type"].asString()), value["section"].asString()) {
	this->url = value["url"].asString();
}

void ExtraSource::print(std::ostream& stream) const {
    Source::print(stream);

    stream << "Url: " << this->url << std::endl;
}
