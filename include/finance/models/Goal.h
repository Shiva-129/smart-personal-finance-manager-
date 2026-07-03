#ifndef FINANCE_MODELS_GOAL_H
#define FINANCE_MODELS_GOAL_H

#include <nlohmann/json.hpp>
#include <string>

namespace finance::models {

enum class GoalStatus {
    IN_PROGRESS,
    ACHIEVED,
    CANCELLED
};

/**
 * @brief A savings goal tracked by the user.
 *
 * Example: Buy a laptop for 100,000 INR, currently saved 35,000 INR.
 */
class Goal {
public:
    Goal() = default;
    Goal(std::string id, std::string userId, std::string name,
         double targetAmount, std::string deadline = {});

    // ── Accessors ───────────────────────────────────────────────────
    const std::string& id()           const { return id_; }
    const std::string& userId()       const { return userId_; }
    const std::string& name()         const { return name_; }
    double             targetAmount() const { return targetAmount_; }
    double             currentAmount() const { return currentAmount_; }
    const std::string& deadline()     const { return deadline_; }
    const std::string& createdAt()    const { return createdAt_; }
    GoalStatus         status()       const { return status_; }

    double progressPercentage() const;
    bool   isAchieved()         const { return status_ == GoalStatus::ACHIEVED; }

    // ── Mutators ────────────────────────────────────────────────────
    void setName(const std::string& name)          { name_ = name; }
    void setTargetAmount(double amt)               { targetAmount_ = amt; }
    void setCurrentAmount(double amt)              { currentAmount_ = amt; }
    void addAmount(double amt)                     { currentAmount_ += amt; }
    void setDeadline(const std::string& deadline)  { deadline_ = deadline; }
    void setStatus(GoalStatus s)                   { status_ = s; }
    void markAchieved()                            { status_ = GoalStatus::ACHIEVED; }

    // ── Serialization ───────────────────────────────────────────────
    nlohmann::json toJson() const;
    static Goal fromJson(const nlohmann::json& j);

    static std::string statusToString(GoalStatus s);
    static GoalStatus statusFromString(const std::string& s);

private:
    std::string id_;
    std::string userId_;
    std::string name_;
    double      targetAmount_ = 0.0;
    double      currentAmount_ = 0.0;
    std::string deadline_;
    std::string createdAt_;
    GoalStatus  status_ = GoalStatus::IN_PROGRESS;
};

}  // namespace finance::models

#endif  // FINANCE_MODELS_GOAL_H
