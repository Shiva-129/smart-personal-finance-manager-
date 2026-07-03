#ifndef FINANCE_MODELS_USER_H
#define FINANCE_MODELS_USER_H

#include <nlohmann/json.hpp>
#include <string>

namespace finance::models {

/**
 * @brief Represents a registered user of the application.
 *
 * Each user owns independent accounts, transactions, budgets, and goals.
 */
class User {
public:
    User() = default;
    User(std::string id, std::string username, std::string passwordHash,
         std::string displayName = {});

    // ── Accessors ───────────────────────────────────────────────────
    const std::string& id()            const { return id_; }
    const std::string& username()      const { return username_; }
    const std::string& passwordHash()  const { return passwordHash_; }
    const std::string& displayName()   const { return displayName_; }
    const std::string& createdAt()     const { return createdAt_; }
    const std::string& lastLoginAt()   const { return lastLoginAt_; }
    bool               isActive()      const { return isActive_; }

    // ── Mutators ────────────────────────────────────────────────────
    void setPasswordHash(const std::string& hash) { passwordHash_ = hash; }
    void setDisplayName(const std::string& name)  { displayName_ = name; }
    void setLastLoginAt(const std::string& ts)    { lastLoginAt_ = ts; }
    void setActive(bool active)                   { isActive_ = active; }

    // ── Serialization ───────────────────────────────────────────────
    nlohmann::json toJson() const;
    static User fromJson(const nlohmann::json& j);

private:
    std::string id_;
    std::string username_;
    std::string passwordHash_;
    std::string displayName_;
    std::string createdAt_;
    std::string lastLoginAt_;
    bool        isActive_ = true;
};

}  // namespace finance::models

#endif  // FINANCE_MODELS_USER_H
