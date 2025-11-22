#include "common.hpp"

#include <stdexcept>
#include <random>
#include <cmath>

std::string genRandomDigits(const size_t N) {
    if (N == 0)  return "0";
    if (N > 20)  throw std::invalid_argument("N too large (max 20 for uint64_t)");

    // Minimum: 10^(N-1)   → smallest N-digit number (e.g. 1000 for N=4)
    // Maximum: 10^N - 1   → largest N-digit number   (e.g. 9999 for N=4)
    const uint64_t min = static_cast<uint64_t>(std::pow(10, N - 1));
    const uint64_t max = min * 10 - 1;

    // Thread-local RNG → no locks, perfect for multithreaded code
    thread_local std::mt19937_64 rng{std::random_device{}()};

    // Uniform distribution over the exact range we need
    std::uniform_int_distribution<uint64_t> dist(min, max);

    return std::to_string(dist(rng));
}