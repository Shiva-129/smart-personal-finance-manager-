#include "finance/utils/UUID.h"

#include <array>
#include <cstdio>
#include <random>
#include <regex>

namespace finance::utils {

std::string UUID::generate()
{
    static thread_local std::mt19937_64 rng(std::random_device{}());

    // Generate 128 random bits.
    uint64_t hi = rng();
    uint64_t lo = rng();

    // Set UUID version 4 (4 most-significant bits of byte 6 -> 0100).
    hi &= ~0xf000ULL;   // clear version nibble
    hi |= 0x4000ULL;    // set to version 4

    // Set variant (2 most-significant bits of byte 8 -> 10).
    lo &= ~0xc000000000000000ULL;  // clear variant bits
    lo |= 0x8000000000000000ULL;   // set to RFC 4122 variant

    std::array<char, 37> buf{};
    std::snprintf(buf.data(), buf.size(),
                  "%08lx-%04lx-%04lx-%04lx-%012lx",
                  static_cast<unsigned long>(hi >> 32),
                  static_cast<unsigned long>((hi >> 16) & 0xffff),
                  static_cast<unsigned long>(hi & 0xffff),
                  static_cast<unsigned long>(lo >> 48),
                  static_cast<unsigned long>(lo & 0xffffffffffffULL));
    return buf.data();
}

bool UUID::isValid(const std::string& uuid)
{
    static const std::regex pattern(
        "^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$",
        std::regex::icase);
    return std::regex_match(uuid, pattern);
}

}  // namespace finance::utils
