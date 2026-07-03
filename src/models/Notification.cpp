#include "finance/models/Notification.h"
#include "finance/utils/DateUtils.h"

namespace finance::models {

Notification::Notification(std::string id, std::string userId,
                           NotificationType type, std::string message)
    : id_(std::move(id))
    , userId_(std::move(userId))
    , type_(type)
    , message_(std::move(message))
    , createdAt_(utils::DateUtils::timestamp())
{
}

nlohmann::json Notification::toJson() const
{
    return {
        {"id",         id_},
        {"user_id",    userId_},
        {"type",       typeToString(type_)},
        {"message",    message_},
        {"created_at", createdAt_},
        {"is_read",    isRead_}
    };
}

Notification Notification::fromJson(const nlohmann::json& j)
{
    Notification n;
    n.id_        = j.value("id", "");
    n.userId_    = j.value("user_id", "");
    n.type_      = typeFromString(j.value("type", "info"));
    n.message_   = j.value("message", "");
    n.createdAt_ = j.value("created_at", "");
    n.isRead_    = j.value("is_read", false);
    return n;
}

std::string Notification::typeToString(NotificationType t)
{
    switch (t) {
        case NotificationType::BUDGET_EXCEEDED:  return "budget_exceeded";
        case NotificationType::GOAL_ACHIEVED:    return "goal_achieved";
        case NotificationType::GOAL_PROGRESS:    return "goal_progress";
        case NotificationType::RECURRING_PAYMENT:return "recurring_payment";
        case NotificationType::LOW_BALANCE:      return "low_balance";
        case NotificationType::INFO:             return "info";
    }
    return "info";
}

NotificationType Notification::typeFromString(const std::string& s)
{
    if (s == "budget_exceeded")   return NotificationType::BUDGET_EXCEEDED;
    if (s == "goal_achieved")     return NotificationType::GOAL_ACHIEVED;
    if (s == "goal_progress")     return NotificationType::GOAL_PROGRESS;
    if (s == "recurring_payment") return NotificationType::RECURRING_PAYMENT;
    if (s == "low_balance")       return NotificationType::LOW_BALANCE;
    return NotificationType::INFO;
}

}  // namespace finance::models
