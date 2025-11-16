#ifndef BGP_PARSE_HPP
#define BGP_PARSE_HPP

#include <string>

#include "bgp_trie.hpp"

namespace NetUtils::BGP {
    void parseDump(const std::string& path, NetTypes::TriePair& outPair);
    void parseDumpToCache(const std::string& path);

    const NetTypes::TriePair* getTrieFromCache();
} // namespace NetUtils

#endif // BGP_PARSE_HPP