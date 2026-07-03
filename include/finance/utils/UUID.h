#ifndef FINANCE_UTILS_UUID_H
#define FINANCE_UTILS_UUID_H

#include <string>

namespace finance::utils {

/**
 * @brief Generate a version-4 (random) UUID string.
 *
 * Format: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
 * where y is one of [8, 9, a, b].
 */
class UUID {
public:
    /// Return a new random UUID string.
    static std::string generate();

    /// Check whether a string is a valid UUID.
    static bool isValid(const std::string& uuid);

private:
    UUID() = default;  // purely static
};

}  // namespace finance::utils

#endif  // FINANCE_UTILS_UUID_H
