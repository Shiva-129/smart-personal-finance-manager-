#ifndef FINANCE_MODELS_BUDGET_H
#define FINANCE_MODELS_BUDGET_H

#include <nlohmann/json.hpp>
#include <string>

namespace finance::models {

/**
 * @brief A monthly budget for a specific category.
 *
 * Example: Food budget of 6000 INR, with 4200 already spent.
 */
class Budget {
public:
    Budget() = default;
    Budget(std::string id, std::string categoryId, std::string userId,
           int year, int month, double limitAmount);

    // ── Accessors ───────────────────────────────────────────────────
    const std::string& id()          const { return id_; }
    const std::string& categoryId()  const { return categoryId_; }
    const std::string& userId()      const { return userId_; }
    int                year()        const { return year_; }
    int                month()       const { return month_; }
    double             limitAmount() const { return limitAmount_; }
    double             spentAmount() const { return spentAmount_; }
    double             remaining()   const { return limitAmount_ - spentAmount_; }
    double             percentageUsed() const;

    /// Whether the budget is exceeded.
    bool isExceeded() const { return spentAmount_ > limitAmount_; }

    // ── Mutators ────────────────────────────────────────────────────
    void setLimitAmount(double limit)    { limitAmount_ = limit; }
    void setSpentAmount(double spent)    { spentAmount_ = spent; }
    void addSpending(double amount)      { spentAmount_ += amount; }

    // ── Serialization ───────────────────────────────────────────────
    nlohmann::json toJson() const;
    static Budget fromJson(const nlohmann::json& j);

private:
    std::string id_;
    std::string categoryId_;
    std::string userId_;
    int         year_ = 0;
    int         month_ = 0;
    double      limitAmount_ = 0.0;
    double      spentAmount_ = 0.0;
};

}  // namespace finance::models

#endif  // FINANCE_MODELS_BUDGET_H
