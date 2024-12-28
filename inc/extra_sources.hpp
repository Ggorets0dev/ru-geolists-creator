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

std::string
extraTypeToString(ExtraSource::Type type);

ExtraSource::Type
extraStringToType(std::string_view str);

std::string
extraOperationToString(ExtraSource::Operation type);

ExtraSource::Operation
extraStringToOperation(std::string_view str);


