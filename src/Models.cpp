#include "finance/Models.h"

#include <cstdio>
#include <functional>

namespace finance::models {

// ══════════════════════════════════════════════════════════════════════
// User
// ══════════════════════════════════════════════════════════════════════

User::User(std::string id, std::string username, std::string passwordHash,
           std::string displayName)
    : id_(std::move(id)), username_(std::move(username))
    , passwordHash_(std::move(passwordHash)), displayName_(std::move(displayName))
    , createdAt_(utils::DateUtils::timestamp()) {
    if (displayName_.empty()) displayName_ = username_;
}

nlohmann::json User::toJson() const { return {
    {"id",id_},{"username",username_},{"password_hash",passwordHash_},
    {"display_name",displayName_},{"created_at",createdAt_},
    {"last_login_at",lastLoginAt_},{"is_active",isActive_}}; }

User User::fromJson(const nlohmann::json& j) {
    User u; u.id_=j.value("id",""); u.username_=j.value("username","");
    u.passwordHash_=j.value("password_hash",""); u.displayName_=j.value("display_name","");
    u.createdAt_=j.value("created_at",""); u.lastLoginAt_=j.value("last_login_at","");
    u.isActive_=j.value("is_active",true); return u;
}

// ══════════════════════════════════════════════════════════════════════
// Category
// ══════════════════════════════════════════════════════════════════════

Category::Category(std::string id, std::string name, CategoryType type, bool isDefault)
    : id_(std::move(id)), name_(std::move(name)), type_(type), isDefault_(isDefault) {}

nlohmann::json Category::toJson() const { return {{"id",id_},{"name",name_},{"type",typeToString(type_)},{"is_default",isDefault_}}; }

Category Category::fromJson(const nlohmann::json& j) {
    Category c; c.id_=j.value("id",""); c.name_=j.value("name","");
    c.type_=typeFromString(j.value("type","expense")); c.isDefault_=j.value("is_default",false); return c;
}

static std::string catIdFromName(const std::string& name) {
    char buf[32]; std::snprintf(buf,sizeof(buf),"cat-%016zx",std::hash<std::string>{}(name)); return buf;
}

std::vector<Category> Category::defaults() {
    std::vector<Category> cats;
    auto add=[&](const std::string& n, CategoryType t){ cats.emplace_back(catIdFromName(n),n,t,true); };
    add("Salary",CategoryType::INCOME); add("Investment",CategoryType::INCOME);
    add("Freelance",CategoryType::INCOME); add("Gift",CategoryType::INCOME);
    add("Other Income",CategoryType::INCOME);
    add("Food",CategoryType::EXPENSE); add("Travel",CategoryType::EXPENSE);
    add("Rent",CategoryType::EXPENSE); add("Shopping",CategoryType::EXPENSE);
    add("Medical",CategoryType::EXPENSE); add("Fuel",CategoryType::EXPENSE);
    add("Bills",CategoryType::EXPENSE); add("Entertainment",CategoryType::EXPENSE);
    add("Education",CategoryType::EXPENSE); add("Insurance",CategoryType::EXPENSE);
    add("Other Expense",CategoryType::EXPENSE);
    return cats;
}

std::string Category::typeToString(CategoryType t) {
    switch(t){case CategoryType::INCOME:return"income";case CategoryType::EXPENSE:return"expense";case CategoryType::BOTH:return"both";}return"expense";
}
CategoryType Category::typeFromString(const std::string& s) {
    if(s=="income")return CategoryType::INCOME;if(s=="both")return CategoryType::BOTH;return CategoryType::EXPENSE;
}

// ══════════════════════════════════════════════════════════════════════
// Account
// ══════════════════════════════════════════════════════════════════════

Account::Account(std::string id, std::string name, const std::string& currency, const std::string& userId)
    : id_(std::move(id)), name_(std::move(name)), currency_(currency), userId_(userId)
    , createdAt_(utils::DateUtils::timestamp()) {}

void Account::deposit(double a) {
    if(a<=0) throw utils::InvalidInputException("Deposit amount must be positive");
    if(status_==AccountStatus::CLOSED) throw utils::FinanceException("Account is closed");
    if(status_==AccountStatus::FROZEN) throw utils::FinanceException("Account is frozen");
    balance_+=a;
}

void Account::withdraw(double a) {
    if(a<=0) throw utils::InvalidInputException("Withdrawal amount must be positive");
    if(status_==AccountStatus::CLOSED) throw utils::FinanceException("Account is closed");
    if(status_==AccountStatus::FROZEN) throw utils::FinanceException("Account is frozen");
    if(a>balance_) throw utils::NegativeBalanceException(name_);
    balance_-=a;
}

void Account::freeze(){status_=AccountStatus::FROZEN;}
void Account::close(){status_=AccountStatus::CLOSED;}

nlohmann::json Account::toJson() const { return {
    {"id",id_},{"name",name_},{"balance",balance_},{"currency",currency_},
    {"user_id",userId_},{"created_at",createdAt_},{"status",accountStatusToString(status_)},{"type",accountTypeName()}}; }

std::unique_ptr<Account> Account::fromJson(const nlohmann::json& j) {
    std::string t=j.value("type","Cash");
    std::unique_ptr<Account> a;
    if(t=="Cash") a=std::make_unique<CashAccount>();
    else if(t=="Savings") a=std::make_unique<SavingsAccount>();
    else if(t=="Current") a=std::make_unique<CurrentAccount>();
    else if(t=="Credit Card") a=std::make_unique<CreditCardAccount>();
    else if(t=="Wallet") a=std::make_unique<WalletAccount>();
    else a=std::make_unique<CashAccount>();
    a->id_=j.value("id",""); a->name_=j.value("name",""); a->balance_=j.value("balance",0.0);
    a->currency_=j.value("currency","INR"); a->userId_=j.value("user_id",""); a->createdAt_=j.value("created_at","");
    a->status_=accountStatusFromString(j.value("status","active"));
    a->loadDerivedFields(j);
    return a;
}

// CashAccount
std::unique_ptr<Account> CashAccount::clone() const { return std::make_unique<CashAccount>(*this); }
nlohmann::json CashAccount::toJson() const { auto j=Account::toJson(); j["type"]="Cash"; return j; }

// SavingsAccount
std::unique_ptr<Account> SavingsAccount::clone() const { return std::make_unique<SavingsAccount>(*this); }
nlohmann::json SavingsAccount::toJson() const { auto j=Account::toJson(); j["type"]="Savings"; j["interest_rate"]=interestRate_; return j; }
void SavingsAccount::loadDerivedFields(const nlohmann::json& j) { if(j.contains("interest_rate")) interestRate_=j["interest_rate"].get<double>(); }

// CurrentAccount
std::unique_ptr<Account> CurrentAccount::clone() const { return std::make_unique<CurrentAccount>(*this); }
nlohmann::json CurrentAccount::toJson() const { auto j=Account::toJson(); j["type"]="Current"; return j; }

// CreditCardAccount
std::unique_ptr<Account> CreditCardAccount::clone() const { return std::make_unique<CreditCardAccount>(*this); }
nlohmann::json CreditCardAccount::toJson() const { auto j=Account::toJson(); j["type"]="Credit Card"; j["credit_limit"]=creditLimit_; j["due_amount"]=dueAmount_; j["due_date"]=dueDate_; return j; }
void CreditCardAccount::loadDerivedFields(const nlohmann::json& j) {
    if(j.contains("credit_limit")) creditLimit_=j["credit_limit"].get<double>();
    if(j.contains("due_amount")) dueAmount_=j["due_amount"].get<double>();
    if(j.contains("due_date")) dueDate_=j["due_date"].get<std::string>();
}
void CreditCardAccount::withdraw(double a) {
    if(a<=0) throw utils::InvalidInputException("Withdrawal must be positive");
    if(status_==AccountStatus::CLOSED) throw utils::FinanceException("Account is closed");
    if(status_==AccountStatus::FROZEN) throw utils::FinanceException("Account is frozen");
    if(balance_-a<-creditLimit_) throw utils::CreditLimitException(name_);
    balance_-=a;
}

// WalletAccount
std::unique_ptr<Account> WalletAccount::clone() const { return std::make_unique<WalletAccount>(*this); }
nlohmann::json WalletAccount::toJson() const { auto j=Account::toJson(); j["type"]="Wallet"; return j; }

std::string accountStatusToString(AccountStatus s) {
    switch(s){case AccountStatus::ACTIVE:return"active";case AccountStatus::FROZEN:return"frozen";case AccountStatus::CLOSED:return"closed";}return"active";
}
AccountStatus accountStatusFromString(const std::string& s) {
    if(s=="frozen")return AccountStatus::FROZEN;if(s=="closed")return AccountStatus::CLOSED;return AccountStatus::ACTIVE;
}

// ══════════════════════════════════════════════════════════════════════
// Transaction
// ══════════════════════════════════════════════════════════════════════

Transaction::Transaction(std::string id, double amount, std::string description,
    std::string categoryId, std::string accountId, std::string userId,
    std::string date, std::string time)
    : id_(std::move(id)), description_(std::move(description)), categoryId_(std::move(categoryId))
    , accountId_(std::move(accountId)), userId_(std::move(userId)), date_(std::move(date)), time_(std::move(time))
    , amount_(amount) {
    if(date_.empty())date_=utils::DateUtils::today();
    if(time_.empty())time_=utils::DateUtils::now();
}

nlohmann::json Transaction::toJson() const { return {
    {"id",id_},{"amount",amount_},{"description",description_},{"category_id",categoryId_},
    {"account_id",accountId_},{"user_id",userId_},{"date",date_},{"time",time_},
    {"tags",tags_},{"notes",notes_},{"type",transactionTypeName()}}; }

std::unique_ptr<Transaction> Transaction::fromJson(const nlohmann::json& j) {
    std::string t=j.value("type","Expense");
    std::unique_ptr<Transaction> txn;
    if(t=="Income") txn=std::make_unique<Income>();
    else if(t=="Expense") txn=std::make_unique<Expense>();
    else if(t=="Transfer") txn=std::make_unique<Transfer>();
    else txn=std::make_unique<Expense>();
    txn->id_=j.value("id",""); txn->amount_=j.value("amount",0.0); txn->description_=j.value("description","");
    txn->categoryId_=j.value("category_id",""); txn->accountId_=j.value("account_id","");
    txn->userId_=j.value("user_id",""); txn->date_=j.value("date",""); txn->time_=j.value("time","");
    txn->tags_=j.value("tags",nlohmann::json::array()).get<std::vector<std::string>>();
    txn->notes_=j.value("notes","");
    txn->loadDerivedFields(j);
    return txn;
}

// Income
std::unique_ptr<Transaction> Income::clone() const { return std::make_unique<Income>(*this); }
nlohmann::json Income::toJson() const { auto j=Transaction::toJson(); j["type"]="Income"; return j; }

// Expense
std::unique_ptr<Transaction> Expense::clone() const { return std::make_unique<Expense>(*this); }
nlohmann::json Expense::toJson() const { auto j=Transaction::toJson(); j["type"]="Expense"; j["is_recurring"]=isRecurring_; j["recurrence_rule"]=recurrenceRule_; return j; }
void Expense::loadDerivedFields(const nlohmann::json& j) { isRecurring_=j.value("is_recurring",false); recurrenceRule_=j.value("recurrence_rule",""); }

// Transfer
std::unique_ptr<Transaction> Transfer::clone() const { return std::make_unique<Transfer>(*this); }
nlohmann::json Transfer::toJson() const { auto j=Transaction::toJson(); j["type"]="Transfer"; j["to_account_id"]=toAccountId_; return j; }
void Transfer::loadDerivedFields(const nlohmann::json& j) { if(j.contains("to_account_id")) toAccountId_=j["to_account_id"].get<std::string>(); }

// ══════════════════════════════════════════════════════════════════════
// Budget
// ══════════════════════════════════════════════════════════════════════

Budget::Budget(std::string id, std::string categoryId, std::string userId, int year, int month, double limitAmount)
    : id_(std::move(id)), categoryId_(std::move(categoryId)), userId_(std::move(userId))
    , year_(year), month_(month), limitAmount_(limitAmount) {}

double Budget::percentageUsed() const { return (limitAmount_<=0.0)?0.0:(spentAmount_/limitAmount_)*100.0; }

nlohmann::json Budget::toJson() const { return {
    {"id",id_},{"category_id",categoryId_},{"user_id",userId_},
    {"year",year_},{"month",month_},{"limit_amount",limitAmount_},{"spent_amount",spentAmount_}}; }

Budget Budget::fromJson(const nlohmann::json& j) {
    Budget b; b.id_=j.value("id",""); b.categoryId_=j.value("category_id",""); b.userId_=j.value("user_id","");
    b.year_=j.value("year",0); b.month_=j.value("month",0); b.limitAmount_=j.value("limit_amount",0.0); b.spentAmount_=j.value("spent_amount",0.0); return b;
}

// ══════════════════════════════════════════════════════════════════════
// Goal
// ══════════════════════════════════════════════════════════════════════

Goal::Goal(std::string id, std::string userId, std::string name, double targetAmount, std::string deadline)
    : id_(std::move(id)), userId_(std::move(userId)), name_(std::move(name))
    , targetAmount_(targetAmount), deadline_(std::move(deadline))
    , createdAt_(utils::DateUtils::timestamp()) {}

double Goal::progressPercentage() const { if(targetAmount_<=0.0)return 0.0; double p=(currentAmount_/targetAmount_)*100.0; return (p>100.0)?100.0:p; }

nlohmann::json Goal::toJson() const { return {
    {"id",id_},{"user_id",userId_},{"name",name_},{"target_amount",targetAmount_},
    {"current_amount",currentAmount_},{"deadline",deadline_},{"created_at",createdAt_},{"status",statusToString(status_)} }; }

Goal Goal::fromJson(const nlohmann::json& j) {
    Goal g; g.id_=j.value("id",""); g.userId_=j.value("user_id",""); g.name_=j.value("name","");
    g.targetAmount_=j.value("target_amount",0.0); g.currentAmount_=j.value("current_amount",0.0);
    g.deadline_=j.value("deadline",""); g.createdAt_=j.value("created_at","");
    g.status_=statusFromString(j.value("status","in_progress")); return g;
}

std::string Goal::statusToString(GoalStatus s) {
    switch(s){case GoalStatus::IN_PROGRESS:return"in_progress";case GoalStatus::ACHIEVED:return"achieved";case GoalStatus::CANCELLED:return"cancelled";}return"in_progress";
}
GoalStatus Goal::statusFromString(const std::string& s) {
    if(s=="achieved")return GoalStatus::ACHIEVED;if(s=="cancelled")return GoalStatus::CANCELLED;return GoalStatus::IN_PROGRESS;
}

// ══════════════════════════════════════════════════════════════════════
// Notification
// ══════════════════════════════════════════════════════════════════════

Notification::Notification(std::string id, std::string userId, NotificationType type, std::string message)
    : id_(std::move(id)), userId_(std::move(userId)), type_(type), message_(std::move(message))
    , createdAt_(utils::DateUtils::timestamp()) {}

nlohmann::json Notification::toJson() const { return {
    {"id",id_},{"user_id",userId_},{"type",typeToString(type_)},{"message",message_},{"created_at",createdAt_},{"is_read",isRead_}}; }

Notification Notification::fromJson(const nlohmann::json& j) {
    Notification n; n.id_=j.value("id",""); n.userId_=j.value("user_id","");
    n.type_=typeFromString(j.value("type","info")); n.message_=j.value("message","");
    n.createdAt_=j.value("created_at",""); n.isRead_=j.value("is_read",false); return n;
}

std::string Notification::typeToString(NotificationType t) {
    switch(t){case NotificationType::BUDGET_EXCEEDED:return"budget_exceeded";case NotificationType::GOAL_ACHIEVED:return"goal_achieved";
    case NotificationType::GOAL_PROGRESS:return"goal_progress";case NotificationType::RECURRING_PAYMENT:return"recurring_payment";
    case NotificationType::LOW_BALANCE:return"low_balance";case NotificationType::INFO:return"info";}return"info";
}
NotificationType Notification::typeFromString(const std::string& s) {
    if(s=="budget_exceeded")return NotificationType::BUDGET_EXCEEDED;if(s=="goal_achieved")return NotificationType::GOAL_ACHIEVED;
    if(s=="goal_progress")return NotificationType::GOAL_PROGRESS;if(s=="recurring_payment")return NotificationType::RECURRING_PAYMENT;
    if(s=="low_balance")return NotificationType::LOW_BALANCE;return NotificationType::INFO;
}

}  // namespace finance::models
