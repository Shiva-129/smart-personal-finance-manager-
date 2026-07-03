#include "finance/services/AuthenticationService.h"
#include "finance/utils/DateUtils.h"
#include "finance/utils/Exceptions.h"
#include "finance/utils/UUID.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace finance::services {

AuthenticationService::AuthenticationService(
    storage::IRepository<models::User>& userRepo)
    : userRepo_(userRepo)
{
}

// ── Password hashing (demonstration only) ────────────────────────────
// In production, replace with bcrypt / argon2 / scrypt.

static std::string toHex(std::size_t value)
{
    std::ostringstream oss;
    oss << std::hex << value;
    return oss.str();
}

std::string AuthenticationService::hashPassword(
    const std::string& password, const std::string& salt)
{
    // Combine password and salt, hash with std::hash.
    std::string combined = password + "::" + salt;
    std::size_t h = std::hash<std::string>{}(combined);
    return toHex(h);
}

bool AuthenticationService::verifyPassword(
    const std::string& password, const std::string& salt,
    const std::string& storedHash)
{
    return hashPassword(password, salt) == storedHash;
}

// ── Core operations ─────────────────────────────────────────────────

models::User AuthenticationService::registerUser(
    const std::string& username, const std::string& password,
    const std::string& displayName)
{
    if (username.empty()) {
        throw utils::InvalidInputException("Username cannot be empty");
    }
    if (password.empty()) {
        throw utils::InvalidInputException("Password cannot be empty");
    }
    if (password.length() < 4) {
        throw utils::InvalidInputException("Password must be at least 4 characters");
    }

    // Check for duplicate username.
    auto existing = userRepo_.find(
        [&](const models::User& u) { return u.username() == username; });
    if (!existing.empty()) {
        throw utils::DuplicateException("User", username);
    }

    std::string salt = utils::UUID::generate();
    std::string hash = hashPassword(password, salt);
    // Store both salt and hash in the password_hash field (salt:hash).
    std::string stored = salt + ":" + hash;

    std::string id = utils::UUID::generate();
    models::User user(id, username, stored, displayName);
    userRepo_.save(user);

    return user;
}

models::User AuthenticationService::login(
    const std::string& username, const std::string& password)
{
    auto users = userRepo_.find(
        [&](const models::User& u) { return u.username() == username; });

    if (users.empty()) {
        throw utils::AuthenticationException("User not found: " + username);
    }

    auto& user = users.front();
    if (!user.isActive()) {
        throw utils::AuthenticationException("Account is deactivated");
    }

    // Parse stored salt:hash.
    auto stored = user.passwordHash();
    auto sep = stored.find(':');
    if (sep == std::string::npos) {
        throw utils::CorruptedDataException("Invalid password hash format");
    }
    std::string salt = stored.substr(0, sep);
    std::string hash = stored.substr(sep + 1);

    if (!verifyPassword(password, salt, hash)) {
        throw utils::AuthenticationException("Incorrect password");
    }

    user.setLastLoginAt(utils::DateUtils::timestamp());
    userRepo_.save(user);
    currentUser_ = user;

    return *currentUser_;
}

void AuthenticationService::changePassword(
    const std::string& oldPassword, const std::string& newPassword)
{
    if (!currentUser_) {
        throw utils::AuthenticationException("Not logged in");
    }

    auto user = *currentUser_;
    auto stored = user.passwordHash();
    auto sep = stored.find(':');
    std::string salt = stored.substr(0, sep);
    std::string hash = stored.substr(sep + 1);

    if (!verifyPassword(oldPassword, salt, hash)) {
        throw utils::AuthenticationException("Current password is incorrect");
    }

    if (newPassword.length() < 4) {
        throw utils::InvalidInputException("New password must be at least 4 characters");
    }

    std::string newSalt = utils::UUID::generate();
    std::string newHash = hashPassword(newPassword, newSalt);
    user.setPasswordHash(newSalt + ":" + newHash);
    userRepo_.save(user);
    currentUser_ = user;
}

void AuthenticationService::logout()
{
    currentUser_.reset();
}

}  // namespace finance::services
