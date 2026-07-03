#ifndef FINANCE_SERVICES_GOALSERVICE_H
#define FINANCE_SERVICES_GOALSERVICE_H

#include "finance/models/Goal.h"
#include "finance/models/Transaction.h"
#include "finance/services/NotificationService.h"
#include "finance/storage/IRepository.h"

#include <string>
#include <vector>

namespace finance::services {

/**
 * @brief Manages savings goals and progress tracking.
 *
 * Acts as an Observer: when TransactionService records an income
 * transaction it may call contributeToGoal() to allocate funds.
 */
class GoalService {
public:
    GoalService(storage::IRepository<models::Goal>& goalRepo,
                 NotificationService& notificationService);

    /// Create a new savings goal.
    models::Goal createGoal(const std::string& userId,
                             const std::string& name,
                             double targetAmount,
                             const std::string& deadline = {});

    /// Contribute money toward a goal.  Fires notification if achieved.
    void contribute(const std::string& goalId, double amount);

    /// Delete a goal.
    void removeGoal(const std::string& goalId);

    /// Get all goals for a user.
    std::vector<models::Goal> getGoals(const std::string& userId) const;

    /// Get a single goal by id.
    std::optional<models::Goal> getGoal(const std::string& goalId) const;

private:
    storage::IRepository<models::Goal>& goalRepo_;
    NotificationService&                notificationService_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_GOALSERVICE_H
