#include "finance/models/User.h"
#include "finance/utils/DateUtils.h"

namespace finance::models {

User::User(std::string id, std::string username, std::string passwordHash,
           std::string displayName)
    : id_(std::move(id))
    , username_(std::move(username))
    , passwordHash_(std::move(passwordHash))
    , displayName_(std::move(displayName))
    , createdAt_(utils::DateUtils::timestamp())
{
    if (displayName_.empty()) {
        displayName_ = username_;
    }
}

nlohmann::json User::toJson() const
{
    return {
        {"id",             id_},
        {"username",       username_},
        {"password_hash",  passwordHash_},
        {"display_name",   displayName_},
        {"created_at",     createdAt_},
        {"last_login_at",  lastLoginAt_},
        {"is_active",      isActive_}
    };
}

User User::fromJson(const nlohmann::json& j)
{
    User u;
    u.id_           = j.value("id", "");
    u.username_     = j.value("username", "");
    u.passwordHash_ = j.value("password_hash", "");
    u.displayName_  = j.value("display_name", "");
    u.createdAt_    = j.value("created_at", "");
    u.lastLoginAt_  = j.value("last_login_at", "");
    u.isActive_     = j.value("is_active", true);
    return u;
}

}  // namespace finance::models
