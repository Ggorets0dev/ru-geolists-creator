#ifndef EXTRA_SOURCES_HPP
#define EXTRA_SOURCES_HPP

#include <string>

#include "json_io.hpp"
#include "main_sources.hpp"

class ExtraSource : public Source {

public:
    ExtraSource(const Json::Value& value);
    std::string url;
};

#endif // EXTRA_SOURCES_HPP
