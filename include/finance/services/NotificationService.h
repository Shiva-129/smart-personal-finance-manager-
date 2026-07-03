#ifndef FINANCE_SERVICES_NOTIFICATIONSERVICE_H
#define FINANCE_SERVICES_NOTIFICATIONSERVICE_H

#include "finance/models/Notification.h"
#include "finance/storage/IRepository.h"

#include <string>
#include <vector>

namespace finance::services {

/**
 * @brief Manages user notifications and alerts.
 *
 * Other services (BudgetService, GoalService) call notify() to
 * create alerts that the user can view via the command interface.
 */
class NotificationService {
public:
    explicit NotificationService(
        storage::IRepository<models::Notification>& repo);

    /// Create a notification for a user.
    void notify(const std::string& userId,
                models::NotificationType type,
                const std::string& message);

    /// Get all notifications for a user (most recent first).
    std::vector<models::Notification> getNotifications(
        const std::string& userId) const;

    /// Get only unread notifications.
    std::vector<models::Notification> getUnread(
        const std::string& userId) const;

    /// Mark a single notification as read.
    void markAsRead(const std::string& notificationId);

    /// Mark all of a user's notifications as read.
    void markAllAsRead(const std::string& userId);

    /// Number of unread notifications for a user.
    size_t unreadCount(const std::string& userId) const;

private:
    storage::IRepository<models::Notification>& repo_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_NOTIFICATIONSERVICE_H
