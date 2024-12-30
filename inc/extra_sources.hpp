#include <string>

#include "json_io.hpp"
#include "main_sources.hpp"

class ExtraSource : public Source {

public:
    ExtraSource(const Json::Value& value);

    std::string url;
};
