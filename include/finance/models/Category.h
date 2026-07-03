#ifndef FINANCE_MODELS_CATEGORY_H
#define FINANCE_MODELS_CATEGORY_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace finance::models {

/// Whether the category applies to income, expense, or both.
enum class CategoryType {
    INCOME,
    EXPENSE,
    BOTH
};

/**
 * @brief A spending or income category.
 *
 * Some categories are built-in defaults; users can also create custom ones.
 */
class Category {
public:
    Category() = default;
    Category(std::string id, std::string name, CategoryType type,
             bool isDefault = false);

    // ── Accessors ───────────────────────────────────────────────────
    const std::string& id()        const { return id_; }
    const std::string& name()      const { return name_; }
    CategoryType       type()      const { return type_; }
    bool               isDefault() const { return isDefault_; }

    // ── Mutators ────────────────────────────────────────────────────
    void setName(const std::string& name) { name_ = name; }
    void setType(CategoryType type)       { type_ = type; }

    // ── Serialization ───────────────────────────────────────────────
    nlohmann::json toJson() const;
    static Category fromJson(const nlohmann::json& j);

    /// Return the built-in default categories.
    static std::vector<Category> defaults();

    /// Convert CategoryType to / from string.
    static std::string typeToString(CategoryType t);
    static CategoryType typeFromString(const std::string& s);

private:
    std::string  id_;
    std::string  name_;
    CategoryType type_ = CategoryType::EXPENSE;
    bool         isDefault_ = false;
};

}  // namespace finance::models

#endif  // FINANCE_MODELS_CATEGORY_H
