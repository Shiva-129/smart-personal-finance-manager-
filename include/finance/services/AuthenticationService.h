#ifndef FINANCE_SERVICES_AUTHENTICATIONSERVICE_H
#define FINANCE_SERVICES_AUTHENTICATIONSERVICE_H

#include "finance/models/User.h"
#include "finance/storage/IRepository.h"

#include <optional>
#include <string>

namespace finance::services {

/**
 * @brief Handles user registration, login, and password management.
 *
 * Uses a simple salted hash for passwords.  In production this should
 * be replaced with bcrypt / argon2.
 */
class AuthenticationService {
public:
    explicit AuthenticationService(
        storage::IRepository<models::User>& userRepo);

    /// Register a new user.  Throws on duplicate username.
    models::User registerUser(const std::string& username,
                               const std::string& password,
                               const std::string& displayName = {});

    /// Log in with username and password.  Throws on failure.
    models::User login(const std::string& username,
                        const std::string& password);

    /// Change password for the currently logged-in user.
    void changePassword(const std::string& oldPassword,
                         const std::string& newPassword);

    /// Log out the current user.
    void logout();

    /// Get the currently logged-in user.
    const std::optional<models::User>& currentUser() const { return currentUser_; }

    /// Whether a user is currently logged in.
    bool isLoggedIn() const { return currentUser_.has_value(); }

private:
    /// Simple salted hash (demonstration only — not cryptographically secure).
    static std::string hashPassword(const std::string& password,
                                     const std::string& salt);
    static bool verifyPassword(const std::string& password,
                                const std::string& salt,
                                const std::string& storedHash);

    storage::IRepository<models::User>& userRepo_;
    std::optional<models::User>         currentUser_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_AUTHENTICATIONSERVICE_H
