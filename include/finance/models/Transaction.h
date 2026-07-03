#ifndef FINANCE_MODELS_TRANSACTION_H
#define FINANCE_MODELS_TRANSACTION_H

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace finance::models {

/**
 * @brief Abstract base class for all financial transactions.
 */
class Transaction {
public:
    Transaction() = default;
    Transaction(std::string id, double amount, std::string description,
                std::string categoryId, std::string accountId,
                std::string userId, std::string date = {},
                std::string time = {});
    virtual ~Transaction() = default;

    // ── Accessors ───────────────────────────────────────────────────
    const std::string& id()           const { return id_; }
    double             amount()       const { return amount_; }
    const std::string& description()  const { return description_; }
    const std::string& categoryId()   const { return categoryId_; }
    const std::string& accountId()    const { return accountId_; }
    const std::string& userId()       const { return userId_; }
    const std::string& date()         const { return date_; }
    const std::string& time()         const { return time_; }
    const std::vector<std::string>& tags() const { return tags_; }
    const std::string& notes()        const { return notes_; }

    // ── Mutators ────────────────────────────────────────────────────
    void setAmount(double a)                { amount_ = a; }
    void setDescription(const std::string& d) { description_ = d; }
    void setCategoryId(const std::string& c) { categoryId_ = c; }
    void setDate(const std::string& d)      { date_ = d; }
    void setTime(const std::string& t)      { time_ = t; }
    void setTags(const std::vector<std::string>& tags) { tags_ = tags; }
    void setNotes(const std::string& n)     { notes_ = n; }

    /// Human-readable transaction type (implemented by each derived class).
    virtual std::string transactionTypeName() const = 0;

    /// Serialization.
    virtual nlohmann::json toJson() const;
    static std::unique_ptr<Transaction> fromJson(const nlohmann::json& j);

    virtual std::unique_ptr<Transaction> clone() const = 0;

    /// Load derived-type-specific fields from JSON (used by fromJson).
    virtual void loadDerivedFields(const nlohmann::json& j) {}

protected:
    std::string             id_;
    double                  amount_ = 0.0;
    std::string             description_;
    std::string             categoryId_;
    std::string             accountId_;
    std::string             userId_;
    std::string             date_;
    std::string             time_;
    std::vector<std::string> tags_;
    std::string             notes_;
};

// ── Concrete Transaction Types ──────────────────────────────────────

class Income : public Transaction {
public:
    using Transaction::Transaction;
    std::string transactionTypeName() const override { return "Income"; }
    std::unique_ptr<Transaction> clone() const override;
    nlohmann::json toJson() const override;
};

class Expense : public Transaction {
public:
    using Transaction::Transaction;
    std::string transactionTypeName() const override { return "Expense"; }
    std::unique_ptr<Transaction> clone() const override;
    nlohmann::json toJson() const override;
    void loadDerivedFields(const nlohmann::json& j) override;

    /// Whether this is a recurring expense.
    bool isRecurring() const { return isRecurring_; }
    void setRecurring(bool r) { isRecurring_ = r; }

    /// Recurrence rule (e.g. "monthly", "yearly").
    std::string recurrenceRule() const { return recurrenceRule_; }
    void setRecurrenceRule(const std::string& rule) { recurrenceRule_ = rule; }

private:
    bool        isRecurring_ = false;
    std::string recurrenceRule_;
};

class Transfer : public Transaction {
public:
    using Transaction::Transaction;
    std::string transactionTypeName() const override { return "Transfer"; }
    std::unique_ptr<Transaction> clone() const override;
    nlohmann::json toJson() const override;
    void loadDerivedFields(const nlohmann::json& j) override;

    const std::string& toAccountId() const { return toAccountId_; }
    void setToAccountId(const std::string& id) { toAccountId_ = id; }

private:
    std::string toAccountId_;
};

}  // namespace finance::models

#endif  // FINANCE_MODELS_TRANSACTION_H
