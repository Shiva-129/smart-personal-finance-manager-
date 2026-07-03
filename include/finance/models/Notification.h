#ifndef FINANCE_MODELS_NOTIFICATION_H
#define FINANCE_MODELS_NOTIFICATION_H

#include <nlohmann/json.hpp>
#include <string>

namespace finance::models {

/// Types of notifications the system can generate.
enum class NotificationType {
    BUDGET_EXCEEDED,
    GOAL_ACHIEVED,
    GOAL_PROGRESS,
    RECURRING_PAYMENT,
    LOW_BALANCE,
    INFO
};

/**
 * @brief A notification or alert for the user.
 */
class Notification {
public:
    Notification() = default;
    Notification(std::string id, std::string userId, NotificationType type,
                 std::string message);

    // ── Accessors ───────────────────────────────────────────────────
    const std::string& id()           const { return id_; }
    const std::string& userId()       const { return userId_; }
    NotificationType   type()         const { return type_; }
    const std::string& message()      const { return message_; }
    const std::string& createdAt()    const { return createdAt_; }
    bool               isRead()       const { return isRead_; }

    // ── Mutators ────────────────────────────────────────────────────
    void markRead() { isRead_ = true; }

    // ── Serialization ───────────────────────────────────────────────
    nlohmann::json toJson() const;
    static Notification fromJson(const nlohmann::json& j);

    static std::string typeToString(NotificationType t);
    static NotificationType typeFromString(const std::string& s);

private:
    std::string      id_;
    std::string      userId_;
    NotificationType type_ = NotificationType::INFO;
    std::string      message_;
    std::string      createdAt_;
    bool             isRead_ = false;
};

}  // namespace finance::models

#endif  // FINANCE_MODELS_NOTIFICATION_H
