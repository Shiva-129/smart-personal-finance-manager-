#ifndef FINANCE_STORAGE_IREPOSITORY_H
#define FINANCE_STORAGE_IREPOSITORY_H

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace finance::storage {

/**
 * @brief Abstract interface for a type-safe repository.
 *
 * Enables swapping between JSON, SQLite, or any other storage backend
 * without changing business-logic code.
 *
 * @tparam T  Model type, which must provide:
 *            - T::fromJson(const nlohmann::json&)
 *            - .toJson() const -> nlohmann::json
 *            - .id() const -> const std::string&
 */
template<typename T>
class IRepository {
public:
    using Predicate = std::function<bool(const T&)>;

    virtual ~IRepository() = default;

    /// Return all items.
    virtual std::vector<T> loadAll() = 0;

    /// Overwrite all items and persist.
    virtual void saveAll(const std::vector<T>& items) = 0;

    /// Find a single item by id.
    virtual std::optional<T> findById(const std::string& id) = 0;

    /// Insert or update a single item.
    virtual void save(const T& item) = 0;

    /// Remove by id.  Returns true if found and removed.
    virtual bool remove(const std::string& id) = 0;

    /// Find items matching a predicate.
    virtual std::vector<T> find(const Predicate& predicate) = 0;

    /// Total item count.
    virtual size_t count() = 0;

    /// Reload from the backing store.
    virtual void reload() = 0;

    /// Force-persist any in-memory changes.
    virtual void flush() = 0;
};

}  // namespace finance::storage

#endif  // FINANCE_STORAGE_IREPOSITORY_H
