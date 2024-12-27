#include <string>

#include "json_io.hpp"

struct ExtraSource {
    enum Type {
        DOMAIN,
        IP
    };

    enum Operation {
        BLOCK,
        PROXY,
        DIRECT
    };

    ExtraSource(const Json::Value& value);

public:
    Type type;
    Operation operation;
    std::string url;
	std::string section;
};
