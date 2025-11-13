#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>
#include <string>
#include <curl/curl.h>

class CurlError : public std::runtime_error {
public:
    CurlError(const std::string& msg, CURLcode code)
        : std::runtime_error(msg + ": " + curl_easy_strerror(code)), errorCode(code) {}

    CURLcode getErrorCode() const { return errorCode; }

private:
    CURLcode errorCode;
};

#endif // EXCEPTION_H
