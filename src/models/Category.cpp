#include "finance/models/Category.h"

#include <cstdio>
#include <functional>

namespace finance::models {

Category::Category(std::string id, std::string name, CategoryType type,
                   bool isDefault)
    : id_(std::move(id))
    , name_(std::move(name))
    , type_(type)
    , isDefault_(isDefault)
{
}

nlohmann::json Category::toJson() const
{
    return {
        {"id",         id_},
        {"name",       name_},
        {"type",       typeToString(type_)},
        {"is_default", isDefault_}
    };
}

Category Category::fromJson(const nlohmann::json& j)
{
    Category c;
    c.id_        = j.value("id", "");
    c.name_      = j.value("name", "");
    c.type_      = typeFromString(j.value("type", "expense"));
    c.isDefault_ = j.value("is_default", false);
    return c;
}

/// Deterministic ID from a category name (repeatable across runs).
static std::string categoryIdFromName(const std::string& name)
{
    // Use a hash of the name to produce a deterministic UUID-like string.
    std::size_t h = std::hash<std::string>{}(name);
    static const char* prefix = "cat-";
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%s%016zx", prefix, h);
    return buf;
}

std::vector<Category> Category::defaults()
{
    std::vector<Category> cats;
    auto add = [&](const std::string& name, CategoryType type) {
        cats.emplace_back(categoryIdFromName(name), name, type, true);
    };

    // Income categories
    add("Salary",       CategoryType::INCOME);
    add("Investment",   CategoryType::INCOME);
    add("Freelance",    CategoryType::INCOME);
    add("Gift",         CategoryType::INCOME);
    add("Other Income", CategoryType::INCOME);

    // Expense categories
    add("Food",         CategoryType::EXPENSE);
    add("Travel",       CategoryType::EXPENSE);
    add("Rent",         CategoryType::EXPENSE);
    add("Shopping",     CategoryType::EXPENSE);
    add("Medical",      CategoryType::EXPENSE);
    add("Fuel",         CategoryType::EXPENSE);
    add("Bills",        CategoryType::EXPENSE);
    add("Entertainment",CategoryType::EXPENSE);
    add("Education",    CategoryType::EXPENSE);
    add("Insurance",    CategoryType::EXPENSE);
    add("Other Expense",CategoryType::EXPENSE);

    return cats;
}

std::string Category::typeToString(CategoryType t)
{
    switch (t) {
        case CategoryType::INCOME:  return "income";
        case CategoryType::EXPENSE: return "expense";
        case CategoryType::BOTH:    return "both";
    }
    return "expense";
}

CategoryType Category::typeFromString(const std::string& s)
{
    if (s == "income") return CategoryType::INCOME;
    if (s == "both")   return CategoryType::BOTH;
    return CategoryType::EXPENSE;
}

}  // namespace finance::models
