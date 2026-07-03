#include "finance/services/SearchEngine.h"
#include "finance/utils/DateUtils.h"

#include <algorithm>
#include <cctype>

namespace finance::services {

SearchEngine::SearchEngine(storage::StorageManager& storage)
    : storage_(storage)
{
}

/// Case-insensitive substring match.
static bool containsIgnoreCase(const std::string& haystack,
                                const std::string& needle)
{
    if (needle.empty()) return true;
    auto it = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char a, char b) { return std::tolower(a) == std::tolower(b); });
    return it != haystack.end();
}

std::vector<models::Transaction*> SearchEngine::search(
    const std::string& userId, const Criteria& criteria) const
{
    auto allTxns = storage_.loadTransactions();
    std::vector<models::Transaction*> result;

    for (auto& txn : allTxns) {
        // Filter by user.
        if (txn->userId() != userId) continue;

        // Date range.
        if (criteria.dateFrom && txn->date() < *criteria.dateFrom) continue;
        if (criteria.dateTo && txn->date() > *criteria.dateTo) continue;

        // Amount range.
        if (criteria.amountMin && txn->amount() < *criteria.amountMin) continue;
        if (criteria.amountMax && txn->amount() > *criteria.amountMax) continue;

        // Category.
        if (criteria.categoryId && txn->categoryId() != *criteria.categoryId) continue;

        // Description (substring, case-insensitive).
        if (criteria.description &&
            !containsIgnoreCase(txn->description(), *criteria.description)) continue;

        // Tag.
        if (criteria.tag) {
            auto& tags = txn->tags();
            bool found = std::find(tags.begin(), tags.end(), *criteria.tag) != tags.end();
            if (!found) continue;
        }

        // Account.
        if (criteria.accountId && txn->accountId() != *criteria.accountId) continue;

        // Transaction type.
        if (criteria.transactionType &&
            txn->transactionTypeName() != *criteria.transactionType) continue;

        result.push_back(txn.get());
    }

    return result;
}

}  // namespace finance::services
