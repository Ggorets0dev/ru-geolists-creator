#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>
#include <string>
#include <curl/curl.h>

class CurlError final : public std::runtime_error {
public:
    CurlError(const std::string& msg, const CURLcode code, const long httpCode = 0)
        : std::runtime_error(msg + ": " + curl_easy_strerror(code)), errorCode(code), httpCode(httpCode) {}

    [[nodiscard]]
    CURLcode getErrorCode() const { return errorCode; }

    [[nodiscard]]
    long getHttpCode() const { return httpCode; }

private:
    CURLcode errorCode;
    long httpCode = 0;
};

#endif // EXCEPTION_H
