#ifndef FINANCE_SERVICES_SEARCHENGINE_H
#define FINANCE_SERVICES_SEARCHENGINE_H

#include "finance/models/Transaction.h"
#include "finance/storage/StorageManager.h"

#include <optional>
#include <string>
#include <vector>

namespace finance::services {

/**
 * @brief Multi-criteria search engine for transactions.
 *
 * Supports searching by date range, amount range, category,
 * description (substring), tags, account, and transaction type.
 */
class SearchEngine {
public:
    struct Criteria {
        std::optional<std::string> dateFrom;
        std::optional<std::string> dateTo;
        std::optional<double>      amountMin;
        std::optional<double>      amountMax;
        std::optional<std::string> categoryId;
        std::optional<std::string> description;   // substring match
        std::optional<std::string> tag;
        std::optional<std::string> accountId;
        std::optional<std::string> transactionType; // "Income"/"Expense"/"Transfer"
    };

    explicit SearchEngine(storage::StorageManager& storage);

    /// Search transactions for a user matching all non-null criteria.
    std::vector<models::Transaction*> search(
        const std::string& userId,
        const Criteria& criteria) const;

private:
    storage::StorageManager& storage_;
};

}  // namespace finance::services

#endif  // FINANCE_SERVICES_SEARCHENGINE_H
