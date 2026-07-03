#include "finance/models/Goal.h"
#include "finance/utils/DateUtils.h"

namespace finance::models {

Goal::Goal(std::string id, std::string userId, std::string name,
           double targetAmount, std::string deadline)
    : id_(std::move(id))
    , userId_(std::move(userId))
    , name_(std::move(name))
    , targetAmount_(targetAmount)
    , deadline_(std::move(deadline))
    , createdAt_(utils::DateUtils::timestamp())
{
}

double Goal::progressPercentage() const
{
    if (targetAmount_ <= 0.0) return 0.0;
    double pct = (currentAmount_ / targetAmount_) * 100.0;
    if (pct > 100.0) pct = 100.0;
    return pct;
}

nlohmann::json Goal::toJson() const
{
    return {
        {"id",              id_},
        {"user_id",         userId_},
        {"name",            name_},
        {"target_amount",   targetAmount_},
        {"current_amount",  currentAmount_},
        {"deadline",        deadline_},
        {"created_at",      createdAt_},
        {"status",          statusToString(status_)}
    };
}

Goal Goal::fromJson(const nlohmann::json& j)
{
    Goal g;
    g.id_             = j.value("id", "");
    g.userId_         = j.value("user_id", "");
    g.name_           = j.value("name", "");
    g.targetAmount_   = j.value("target_amount", 0.0);
    g.currentAmount_  = j.value("current_amount", 0.0);
    g.deadline_       = j.value("deadline", "");
    g.createdAt_      = j.value("created_at", "");
    g.status_         = statusFromString(j.value("status", "in_progress"));
    return g;
}

std::string Goal::statusToString(GoalStatus s)
{
    switch (s) {
        case GoalStatus::IN_PROGRESS: return "in_progress";
        case GoalStatus::ACHIEVED:    return "achieved";
        case GoalStatus::CANCELLED:   return "cancelled";
    }
    return "in_progress";
}

GoalStatus Goal::statusFromString(const std::string& s)
{
    if (s == "achieved")  return GoalStatus::ACHIEVED;
    if (s == "cancelled") return GoalStatus::CANCELLED;
    return GoalStatus::IN_PROGRESS;
}

}  // namespace finance::models
