#ifndef FILTER_HPP
#define FILTER_HPP

#include <string>
#include <fs_utils.hpp>

#include "network.hpp"

bool checkAddressByLists(const std::string& addr, const NetTypes::ListIPv4& ipv4, const NetTypes::ListIPv6& ipv6);

bool checkFileByIPvLists(const fs::path& path, const NetTypes::ListIPvxPair& listsPair, bool applyFix);

void parseAddressFile(const fs::path& path, NetTypes::ListIPv4& ipv4, NetTypes::ListIPv6& ipv6);

bool isUrl(const std::string& str);

#endif // FILTER_HPP
