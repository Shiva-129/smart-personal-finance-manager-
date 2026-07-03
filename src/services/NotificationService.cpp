#include "finance/services/NotificationService.h"
#include "finance/utils/DateUtils.h"
#include "finance/utils/UUID.h"

#include <algorithm>

namespace finance::services {

NotificationService::NotificationService(
    storage::IRepository<models::Notification>& repo)
    : repo_(repo)
{
}

void NotificationService::notify(
    const std::string& userId,
    models::NotificationType type,
    const std::string& message)
{
    models::Notification n(utils::UUID::generate(), userId, type, message);
    repo_.save(n);
}

std::vector<models::Notification> NotificationService::getNotifications(
    const std::string& userId) const
{
    auto all = repo_.loadAll();
    std::vector<models::Notification> result;
    std::copy_if(all.begin(), all.end(), std::back_inserter(result),
                 [&](const auto& n) { return n.userId() == userId; });
    // Most recent first.
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) {
                  return a.createdAt() > b.createdAt();
              });
    return result;
}

std::vector<models::Notification> NotificationService::getUnread(
    const std::string& userId) const
{
    auto all = getNotifications(userId);
    std::vector<models::Notification> result;
    std::copy_if(all.begin(), all.end(), std::back_inserter(result),
                 [](const auto& n) { return !n.isRead(); });
    return result;
}

void NotificationService::markAsRead(const std::string& notificationId)
{
    auto opt = repo_.findById(notificationId);
    if (opt) {
        auto n = *opt;
        n.markRead();
        repo_.save(n);
    }
}

void NotificationService::markAllAsRead(const std::string& userId)
{
    auto all = repo_.loadAll();
    for (auto& n : all) {
        if (n.userId() == userId && !n.isRead()) {
            n.markRead();
            repo_.save(n);
        }
    }
}

size_t NotificationService::unreadCount(const std::string& userId) const
{
    return getUnread(userId).size();
}

}  // namespace finance::services
