#include "finance/models/Transaction.h"
#include "finance/utils/DateUtils.h"

namespace finance::models {

// ── Transaction base ────────────────────────────────────────────────

Transaction::Transaction(std::string id, double amount, std::string description,
                         std::string categoryId, std::string accountId,
                         std::string userId, std::string date, std::string time)
    : id_(std::move(id))
    , amount_(amount)
    , description_(std::move(description))
    , categoryId_(std::move(categoryId))
    , accountId_(std::move(accountId))
    , userId_(std::move(userId))
    , date_(std::move(date))
    , time_(std::move(time))
{
    if (date_.empty()) date_ = utils::DateUtils::today();
    if (time_.empty()) time_ = utils::DateUtils::now();
}

nlohmann::json Transaction::toJson() const
{
    return {
        {"id",           id_},
        {"amount",       amount_},
        {"description",  description_},
        {"category_id",  categoryId_},
        {"account_id",   accountId_},
        {"user_id",      userId_},
        {"date",         date_},
        {"time",         time_},
        {"tags",         tags_},
        {"notes",        notes_},
        {"type",         transactionTypeName()}
    };
}

std::unique_ptr<Transaction> Transaction::fromJson(const nlohmann::json& j)
{
    std::string type = j.value("type", "Expense");
    std::unique_ptr<Transaction> txn;

    if (type == "Income")        txn = std::make_unique<Income>();
    else if (type == "Expense")  txn = std::make_unique<Expense>();
    else if (type == "Transfer") txn = std::make_unique<Transfer>();
    else                         txn = std::make_unique<Expense>();

    txn->id_         = j.value("id", "");
    txn->amount_     = j.value("amount", 0.0);
    txn->description_= j.value("description", "");
    txn->categoryId_ = j.value("category_id", "");
    txn->accountId_  = j.value("account_id", "");
    txn->userId_     = j.value("user_id", "");
    txn->date_       = j.value("date", "");
    txn->time_       = j.value("time", "");
    txn->tags_       = j.value("tags", nlohmann::json::array())
                            .get<std::vector<std::string>>();
    txn->notes_      = j.value("notes", "");

    // Derived-type-specific fields via virtual dispatch.
    txn->loadDerivedFields(j);

    return txn;
}

// ── Income ──────────────────────────────────────────────────────────

std::unique_ptr<Transaction> Income::clone() const
{
    return std::make_unique<Income>(*this);
}

nlohmann::json Income::toJson() const
{
    auto j = Transaction::toJson();
    j["type"] = "Income";
    return j;
}

// ── Expense ─────────────────────────────────────────────────────────

std::unique_ptr<Transaction> Expense::clone() const
{
    return std::make_unique<Expense>(*this);
}

nlohmann::json Expense::toJson() const
{
    auto j = Transaction::toJson();
    j["type"]           = "Expense";
    j["is_recurring"]   = isRecurring_;
    j["recurrence_rule"] = recurrenceRule_;
    return j;
}

void Expense::loadDerivedFields(const nlohmann::json& j)
{
    isRecurring_   = j.value("is_recurring", false);
    recurrenceRule_ = j.value("recurrence_rule", "");
}

// ── Transfer ────────────────────────────────────────────────────────

std::unique_ptr<Transaction> Transfer::clone() const
{
    return std::make_unique<Transfer>(*this);
}

nlohmann::json Transfer::toJson() const
{
    auto j = Transaction::toJson();
    j["type"]          = "Transfer";
    j["to_account_id"] = toAccountId_;
    return j;
}

void Transfer::loadDerivedFields(const nlohmann::json& j)
{
    if (j.contains("to_account_id")) {
        toAccountId_ = j["to_account_id"].get<std::string>();
    }
}

}  // namespace finance::models
