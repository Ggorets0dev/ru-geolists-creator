#include "extra_sources.hpp"

ExtraSource::ExtraSource(const Json::Value& value) : Source(sourceStringToType(value["type"].asString()), value["section"].asString()) {
	this->url = value["url"].asString();
}
