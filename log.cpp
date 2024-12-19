#include "log.hpp"

void
log(LogType type, std::string_view msg) {
    std::string prefix;
    prefix.reserve(1);

    switch (type) {
    case LogType::ERROR:
        prefix = ERROR_LOG_PREFIX;
        break;
    case LogType::WARNING:
        prefix = WARNING_LOG_PREFIX;
        break;
    case LogType::INFO:
        prefix = INFO_LOG_PREFIX;
        break;
    }

    std::cout << "[" << prefix << "] " << msg << std::endl;
}

void
log(LogType type, const std::string& msg1, const std::string& msg2) {
    log(type, msg1 + " " + msg2);
}
