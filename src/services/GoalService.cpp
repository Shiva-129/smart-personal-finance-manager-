#include "finance/services/GoalService.h"
#include "finance/utils/Exceptions.h"
#include "finance/utils/UUID.h"

#include <algorithm>

namespace finance::services {

GoalService::GoalService(storage::IRepository<models::Goal>& goalRepo,
                           NotificationService& notificationService)
    : goalRepo_(goalRepo)
    , notificationService_(notificationService)
{
}

models::Goal GoalService::createGoal(
    const std::string& userId, const std::string& name,
    double targetAmount, const std::string& deadline)
{
    if (name.empty()) {
        throw utils::InvalidInputException("Goal name cannot be empty");
    }
    if (targetAmount <= 0) {
        throw utils::InvalidInputException("Target amount must be positive");
    }

    std::string id = utils::UUID::generate();
    models::Goal goal(id, userId, name, targetAmount, deadline);
    goalRepo_.save(goal);
    return goal;
}

void GoalService::contribute(const std::string& goalId, double amount)
{
    if (amount <= 0) {
        throw utils::InvalidInputException("Contribution must be positive");
    }

    auto opt = goalRepo_.findById(goalId);
    if (!opt) {
        throw utils::NotFoundException("Goal", goalId);
    }

    auto goal = *opt;
    goal.addAmount(amount);

    // Check if achieved.
    if (goal.currentAmount() >= goal.targetAmount() && !goal.isAchieved()) {
        goal.markAchieved();
        notificationService_.notify(
            goal.userId(),
            models::NotificationType::GOAL_ACHIEVED,
            "Congratulations! You achieved your goal: " + goal.name());
    } else {
        // Notify progress milestone (every 25%).
        double oldPct = (goal.currentAmount() - amount) / goal.targetAmount() * 100.0;
        double newPct = goal.progressPercentage();
        if (static_cast<int>(oldPct / 25) < static_cast<int>(newPct / 25)) {
            notificationService_.notify(
                goal.userId(),
                models::NotificationType::GOAL_PROGRESS,
                "Goal progress: " + goal.name() + " is " +
                std::to_string(static_cast<int>(newPct)) + "% complete");
        }
    }

    goalRepo_.save(goal);
}

void GoalService::removeGoal(const std::string& goalId)
{
    if (!goalRepo_.remove(goalId)) {
        throw utils::NotFoundException("Goal", goalId);
    }
}

std::vector<models::Goal> GoalService::getGoals(
    const std::string& userId) const
{
    return goalRepo_.find(
        [&](const models::Goal& g) { return g.userId() == userId; });
}

std::optional<models::Goal> GoalService::getGoal(
    const std::string& goalId) const
{
    return goalRepo_.findById(goalId);
}

}  // namespace finance::services
