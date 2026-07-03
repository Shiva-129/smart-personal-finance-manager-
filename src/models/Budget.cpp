#include "finance/models/Budget.h"

namespace finance::models {

Budget::Budget(std::string id, std::string categoryId, std::string userId,
               int year, int month, double limitAmount)
    : id_(std::move(id))
    , categoryId_(std::move(categoryId))
    , userId_(std::move(userId))
    , year_(year)
    , month_(month)
    , limitAmount_(limitAmount)
{
}

double Budget::percentageUsed() const
{
    if (limitAmount_ <= 0.0) return 0.0;
    return (spentAmount_ / limitAmount_) * 100.0;
}

nlohmann::json Budget::toJson() const
{
    return {
        {"id",            id_},
        {"category_id",   categoryId_},
        {"user_id",       userId_},
        {"year",          year_},
        {"month",         month_},
        {"limit_amount",  limitAmount_},
        {"spent_amount",  spentAmount_}
    };
}

Budget Budget::fromJson(const nlohmann::json& j)
{
    Budget b;
    b.id_           = j.value("id", "");
    b.categoryId_   = j.value("category_id", "");
    b.userId_       = j.value("user_id", "");
    b.year_         = j.value("year", 0);
    b.month_        = j.value("month", 0);
    b.limitAmount_  = j.value("limit_amount", 0.0);
    b.spentAmount_  = j.value("spent_amount", 0.0);
    return b;
}

}  // namespace finance::models
