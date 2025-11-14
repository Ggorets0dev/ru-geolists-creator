#ifndef BGP_PARSE_HPP
#define BGP_PARSE_HPP

#include <string>

#include "bgp_trie.hpp"

namespace NetUtils {
    void parse_BGP_dump(const std::string& path, NetTypes::TriePair& outPair);
} // namespace NetUtils

#endif // BGP_PARSE_HPP