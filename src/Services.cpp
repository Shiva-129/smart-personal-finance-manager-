#include "finance/Services.h"
#include "finance/Utils.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <numeric>
#include <sstream>

namespace finance::services {

// ══════════════════════════════════════════════════════════════════════
// NotificationService
// ══════════════════════════════════════════════════════════════════════

NotificationService::NotificationService(storage::IRepository<models::Notification>& r) : repo_(r) {}

void NotificationService::notify(const std::string& userId, models::NotificationType type, const std::string& msg) {
    repo_.save(models::Notification(utils::UUID::generate(), userId, type, msg));
}

std::vector<models::Notification> NotificationService::getNotifications(const std::string& userId) const {
    auto all = repo_.loadAll();
    std::vector<models::Notification> r;
    std::copy_if(all.begin(),all.end(),std::back_inserter(r),[&](const auto& n){return n.userId()==userId;});
    std::sort(r.begin(),r.end(),[](const auto& a,const auto& b){return a.createdAt()>b.createdAt();});
    return r;
}

std::vector<models::Notification> NotificationService::getUnread(const std::string& userId) const {
    auto n = getNotifications(userId); std::vector<models::Notification> r;
    std::copy_if(n.begin(),n.end(),std::back_inserter(r),[](const auto& n){return !n.isRead();}); return r;
}

void NotificationService::markAsRead(const std::string& id) {
    auto o=repo_.findById(id); if(o){auto n=*o; n.markRead(); repo_.save(n);}
}

void NotificationService::markAllAsRead(const std::string& userId) {
    auto all=repo_.loadAll(); for(auto& n:all){if(n.userId()==userId&&!n.isRead()){n.markRead();repo_.save(n);}}
}

size_t NotificationService::unreadCount(const std::string& userId) const { return getUnread(userId).size(); }

// ══════════════════════════════════════════════════════════════════════
// AuthenticationService
// ══════════════════════════════════════════════════════════════════════

AuthenticationService::AuthenticationService(storage::IRepository<models::User>& r) : userRepo_(r) {}

static std::string toHex(std::size_t v) { std::ostringstream oss; oss<<std::hex<<v; return oss.str(); }

std::string AuthenticationService::hashPassword(const std::string& pw, const std::string& salt) {
    return toHex(std::hash<std::string>{}(pw+"::"+salt));
}
bool AuthenticationService::verifyPassword(const std::string& pw, const std::string& salt, const std::string& h) {
    return hashPassword(pw,salt)==h;
}

models::User AuthenticationService::registerUser(const std::string& username, const std::string& password, const std::string& displayName) {
    if(username.empty())throw utils::InvalidInputException("Username required");
    if(password.length()<4)throw utils::InvalidInputException("Password min 4 chars");
    if(!userRepo_.find([&](const models::User& u){return u.username()==username;}).empty())
        throw utils::DuplicateException("User",username);
    std::string salt=utils::UUID::generate(), hash=hashPassword(password,salt);
    models::User user(utils::UUID::generate(),username,salt+":"+hash,displayName.empty()?username:displayName);
    userRepo_.save(user); return user;
}

models::User AuthenticationService::login(const std::string& username, const std::string& password) {
    auto users=userRepo_.find([&](const models::User& u){return u.username()==username;});
    if(users.empty())throw utils::AuthenticationException("User not found: "+username);
    auto& user=users.front();
    if(!user.isActive())throw utils::AuthenticationException("Account deactivated");
    auto stored=user.passwordHash(); auto sep=stored.find(':');
    if(sep==std::string::npos)throw utils::CorruptedDataException("Invalid hash");
    if(!verifyPassword(password,stored.substr(0,sep),stored.substr(sep+1)))
        throw utils::AuthenticationException("Incorrect password");
    user.setLastLoginAt(utils::DateUtils::timestamp()); userRepo_.save(user); currentUser_=user;
    return *currentUser_;
}

void AuthenticationService::changePassword(const std::string& oldPw, const std::string& newPw) {
    if(!currentUser_)throw utils::AuthenticationException("Not logged in");
    auto user=*currentUser_; auto stored=user.passwordHash(); auto sep=stored.find(':');
    if(!verifyPassword(oldPw,stored.substr(0,sep),stored.substr(sep+1)))
        throw utils::AuthenticationException("Current password incorrect");
    if(newPw.length()<4)throw utils::InvalidInputException("New password min 4 chars");
    std::string newSalt=utils::UUID::generate();
    user.setPasswordHash(newSalt+":"+hashPassword(newPw,newSalt)); userRepo_.save(user); currentUser_=user;
}

void AuthenticationService::logout() { currentUser_.reset(); }

// ══════════════════════════════════════════════════════════════════════
// AccountService
// ══════════════════════════════════════════════════════════════════════

AccountService::AccountService(storage::StorageManager& s) : storage_(s) { loadAccounts(); }
void AccountService::loadAccounts() { accounts_ = storage_.loadAccounts(); }
void AccountService::saveAccounts() { storage_.saveAccounts(accounts_); }

static std::unique_ptr<models::Account> makeAccount(const std::string& type, const std::string& id,
    const std::string& name, const std::string& currency, const std::string& userId) {
    if(type=="Cash")return std::make_unique<models::CashAccount>(id,name,currency,userId);
    if(type=="Savings")return std::make_unique<models::SavingsAccount>(id,name,currency,userId);
    if(type=="Current")return std::make_unique<models::CurrentAccount>(id,name,currency,userId);
    if(type=="Credit Card")return std::make_unique<models::CreditCardAccount>(id,name,currency,userId);
    if(type=="Wallet")return std::make_unique<models::WalletAccount>(id,name,currency,userId);
    throw utils::InvalidInputException("Unknown account type: "+type);
}

models::Account* AccountService::createAccount(const std::string& userId, const std::string& type,
    const std::string& name, const std::string& currency) {
    if(name.empty())throw utils::InvalidInputException("Account name required");
    auto acc=makeAccount(type,utils::UUID::generate(),name,currency,userId);
    models::Account* p=acc.get(); accounts_.push_back(std::move(acc)); saveAccounts(); return p;
}

void AccountService::deleteAccount(const std::string& id) {
    auto it=std::find_if(accounts_.begin(),accounts_.end(),[&](const auto& a){return a->id()==id;});
    if(it==accounts_.end())throw utils::NotFoundException("Account",id);
    accounts_.erase(it); saveAccounts();
}

void AccountService::freezeAccount(const std::string& id) {
    auto*a=getAccount(id); if(!a)throw utils::NotFoundException("Account",id); a->freeze(); saveAccounts();
}
void AccountService::closeAccount(const std::string& id) {
    auto*a=getAccount(id); if(!a)throw utils::NotFoundException("Account",id); a->close(); saveAccounts();
}

void AccountService::transfer(const std::string& fromId, const std::string& toId, double amt, const std::string& desc) {
    if(amt<=0)throw utils::InvalidInputException("Transfer amount must be positive");
    if(fromId==toId)throw utils::InvalidInputException("Cannot transfer to same account");
    auto*from=getAccount(fromId); auto*to=getAccount(toId);
    if(!from)throw utils::NotFoundException("Source account",fromId);
    if(!to)throw utils::NotFoundException("Dest account",toId);
    from->withdraw(amt); to->deposit(amt); saveAccounts();
}

std::vector<models::Account*> AccountService::getAccountsByUser(const std::string& userId) {
    std::vector<models::Account*> r;
    for(auto& a:accounts_) if(a->userId()==userId) r.push_back(a.get());
    return r;
}

models::Account* AccountService::getAccount(const std::string& id) {
    auto it=std::find_if(accounts_.begin(),accounts_.end(),[&](const auto& a){return a->id()==id;});
    return (it!=accounts_.end())?it->get():nullptr;
}

const std::vector<std::unique_ptr<models::Account>>& AccountService::allAccounts() const { return accounts_; }

// ══════════════════════════════════════════════════════════════════════
// BudgetService
// ══════════════════════════════════════════════════════════════════════

BudgetService::BudgetService(storage::StorageManager& s, NotificationService& ns)
    : storage_(s), notificationService_(ns) {}

models::Budget BudgetService::setBudget(const std::string& userId, const std::string& catId,
    int year, int month, double limit) {
    if(limit<=0)throw utils::InvalidInputException("Budget must be positive");
    if(month<1||month>12)throw utils::InvalidInputException("Invalid month");
    auto exist=storage_.budgets().find([&](const models::Budget& b){return b.userId()==userId&&b.categoryId()==catId&&b.year()==year&&b.month()==month;});
    if(!exist.empty()){auto b=exist.front();b.setLimitAmount(limit);storage_.budgets().save(b);return b;}
    models::Budget b(utils::UUID::generate(),catId,userId,year,month,limit); storage_.budgets().save(b); return b;
}

void BudgetService::removeBudget(const std::string& id) { if(!storage_.budgets().remove(id)) throw utils::NotFoundException("Budget",id); }

void BudgetService::checkBudget(const models::Transaction& txn) {
    auto[y,m,d]=utils::DateUtils::extractYMD(txn.date());
    auto opt=getBudget(txn.userId(),txn.categoryId(),y,m); if(!opt)return;
    auto b=*opt; b.addSpending(txn.amount()); storage_.budgets().save(b);
    if(b.isExceeded()){
        std::string msg="Budget exceeded for category "+txn.categoryId()+": "+std::to_string((int)b.percentageUsed())+"% used";
        notificationService_.notify(txn.userId(),models::NotificationType::BUDGET_EXCEEDED,msg);
    } else if(b.percentageUsed()>=80.0){
        std::string msg="Budget warning for category "+txn.categoryId()+": "+std::to_string((int)b.percentageUsed())+"% used";
        notificationService_.notify(txn.userId(),models::NotificationType::BUDGET_EXCEEDED,msg);
    }
}

std::vector<models::Budget> BudgetService::getBudgets(const std::string& userId, int year, int month) const {
    return storage_.budgets().find([&](const models::Budget& b){return b.userId()==userId&&b.year()==year&&b.month()==month;});
}

std::optional<models::Budget> BudgetService::getBudget(const std::string& userId, const std::string& catId, int year, int month) const {
    auto r=storage_.budgets().find([&](const models::Budget& b){return b.userId()==userId&&b.categoryId()==catId&&b.year()==year&&b.month()==month;});
    return r.empty()?std::nullopt:std::optional(r.front());
}

void BudgetService::recalculate(const std::string& userId, int year, int month) {
    auto budgets=getBudgets(userId,year,month);
    auto all=storage_.loadTransactions();
    for(auto& txn:all){
        auto[y,m,d]=utils::DateUtils::extractYMD(txn->date());
        if(txn->userId()==userId&&txn->transactionTypeName()=="Expense"&&y==year&&m==month)
            for(auto& b:budgets) if(b.categoryId()==txn->categoryId()) b.addSpending(txn->amount());
    }
    for(auto& b:budgets) storage_.budgets().save(b);
}

// ══════════════════════════════════════════════════════════════════════
// GoalService
// ══════════════════════════════════════════════════════════════════════

GoalService::GoalService(storage::IRepository<models::Goal>& r, NotificationService& ns)
    : goalRepo_(r), notificationService_(ns) {}

models::Goal GoalService::createGoal(const std::string& userId, const std::string& name, double target, const std::string& deadline) {
    if(name.empty())throw utils::InvalidInputException("Goal name required");
    if(target<=0)throw utils::InvalidInputException("Target must be positive");
    models::Goal g(utils::UUID::generate(),userId,name,target,deadline); goalRepo_.save(g); return g;
}

void GoalService::contribute(const std::string& goalId, double amount) {
    if(amount<=0)throw utils::InvalidInputException("Contribution must be positive");
    auto opt=goalRepo_.findById(goalId); if(!opt)throw utils::NotFoundException("Goal",goalId);
    auto g=*opt; double oldPct=g.progressPercentage(); g.addAmount(amount);
    if(g.currentAmount()>=g.targetAmount()&&!g.isAchieved()){
        g.markAchieved();
        notificationService_.notify(g.userId(),models::NotificationType::GOAL_ACHIEVED,"Achieved goal: "+g.name());
    } else {
        double newPct=g.progressPercentage();
        if((int)(oldPct/25)<(int)(newPct/25))
            notificationService_.notify(g.userId(),models::NotificationType::GOAL_PROGRESS,g.name()+" is "+std::to_string((int)newPct)+"% complete");
    }
    goalRepo_.save(g);
}

void GoalService::removeGoal(const std::string& id) { if(!goalRepo_.remove(id)) throw utils::NotFoundException("Goal",id); }
std::vector<models::Goal> GoalService::getGoals(const std::string& userId) const { return goalRepo_.find([&](const models::Goal& g){return g.userId()==userId;}); }
std::optional<models::Goal> GoalService::getGoal(const std::string& id) const { return goalRepo_.findById(id); }

// ══════════════════════════════════════════════════════════════════════
// TransactionService
// ══════════════════════════════════════════════════════════════════════

TransactionService::TransactionService(storage::StorageManager& s, AccountService& as,
    BudgetService& bs, GoalService& gs, NotificationService& ns)
    : storage_(s), accountService_(as), budgetService_(bs), goalService_(gs), notificationService_(ns) { loadTransactions(); }

void TransactionService::loadTransactions() { txns_=storage_.loadTransactions(); }
void TransactionService::saveTransactions() { storage_.saveTransactions(txns_); }

models::Transaction* TransactionService::addIncome(const std::string& userId, double amt,
    const std::string& desc, const std::string& catId, const std::string& accId,
    const std::string& date, const std::string& goalId, const std::vector<std::string>& tags) {
    if(amt<=0)throw utils::InvalidInputException("Income must be positive");
    auto*a=accountService_.getAccount(accId); if(!a)throw utils::NotFoundException("Account",accId);
    a->deposit(amt); accountService_.saveAccounts();
    auto t=std::make_unique<models::Income>(utils::UUID::generate(),amt,desc,catId,accId,userId,date);
    t->setTags(tags); models::Transaction*p=t.get(); txns_.push_back(std::move(t)); saveTransactions();
    if(!goalId.empty()) goalService_.contribute(goalId,amt);
    return p;
}

models::Transaction* TransactionService::addExpense(const std::string& userId, double amt,
    const std::string& desc, const std::string& catId, const std::string& accId,
    const std::string& date, bool recurring, const std::string& rule, const std::vector<std::string>& tags) {
    if(amt<=0)throw utils::InvalidInputException("Expense must be positive");
    auto*a=accountService_.getAccount(accId); if(!a)throw utils::NotFoundException("Account",accId);
    a->withdraw(amt); accountService_.saveAccounts();
    auto t=std::make_unique<models::Expense>(utils::UUID::generate(),amt,desc,catId,accId,userId,date);
    t->setRecurring(recurring); t->setRecurrenceRule(rule); t->setTags(tags);
    models::Transaction*p=t.get(); txns_.push_back(std::move(t)); saveTransactions();
    budgetService_.checkBudget(*p);
    return p;
}

models::Transaction* TransactionService::addTransfer(const std::string& userId, double amt,
    const std::string& desc, const std::string& fromId, const std::string& toId, const std::string& date) {
    if(amt<=0)throw utils::InvalidInputException("Transfer must be positive");
    accountService_.transfer(fromId,toId,amt,desc);
    auto t=std::make_unique<models::Transfer>(utils::UUID::generate(),amt,desc,"",fromId,userId,date);
    t->setToAccountId(toId);
    models::Transaction*p=t.get(); txns_.push_back(std::move(t)); saveTransactions();
    return p;
}

void TransactionService::editTransaction(const std::string& id, double amt, const std::string& desc, const std::string& catId) {
    auto it=std::find_if(txns_.begin(),txns_.end(),[&](const auto& t){return t->id()==id;});
    if(it==txns_.end())throw utils::NotFoundException("Transaction",id);
    (*it)->setAmount(amt); (*it)->setDescription(desc); (*it)->setCategoryId(catId); saveTransactions();
}

void TransactionService::deleteTransaction(const std::string& id) {
    auto it=std::find_if(txns_.begin(),txns_.end(),[&](const auto& t){return t->id()==id;});
    if(it==txns_.end())throw utils::NotFoundException("Transaction",id);
    undoStack_.push(std::move(*it)); txns_.erase(it); saveTransactions();
}

bool TransactionService::undoDelete() {
    if(undoStack_.empty())return false;
    auto t=std::move(undoStack_.top()); undoStack_.pop();
    txns_.push_back(std::move(t)); saveTransactions(); return true;
}

std::vector<models::Transaction*> TransactionService::getTransactionsByUser(const std::string& userId) const {
    std::vector<models::Transaction*> r; for(auto& t:txns_) if(t->userId()==userId) r.push_back(t.get()); return r;
}

std::vector<models::Transaction*> TransactionService::getTransactionsByAccount(const std::string& accId) const {
    std::vector<models::Transaction*> r;
    for(auto& t:txns_){if(t->accountId()==accId)r.push_back(t.get());
        if(t->transactionTypeName()=="Transfer"){auto*tr=static_cast<models::Transfer*>(t.get());if(tr->toAccountId()==accId)r.push_back(t.get());}}
    return r;
}

std::vector<models::Transaction*> TransactionService::getTransactionsByCategory(const std::string& catId) const {
    std::vector<models::Transaction*> r; for(auto& t:txns_) if(t->categoryId()==catId) r.push_back(t.get()); return r;
}

models::Transaction* TransactionService::getTransaction(const std::string& id) const {
    auto it=std::find_if(txns_.begin(),txns_.end(),[&](const auto& t){return t->id()==id;});
    return (it!=txns_.end())?it->get():nullptr;
}

size_t TransactionService::processRecurring() {
    size_t cnt=0; std::string today=utils::DateUtils::today();
    auto[curY,curM,curD]=utils::DateUtils::extractYMD(today);
    for(const auto& txn:txns_){
        if(txn->transactionTypeName()!="Expense")continue;
        auto*exp=static_cast<const models::Expense*>(txn.get());
        if(!exp->isRecurring()||exp->recurrenceRule().empty())continue;
        auto[txY,txM,txD]=utils::DateUtils::extractYMD(txn->date());
        bool gen=false;
        if(exp->recurrenceRule()=="monthly") gen=((curY-txY)*12+(curM-txM)>=1);
        else if(exp->recurrenceRule()=="yearly") gen=(curY>txY&&curM==txM&&curD>=txD);
        else if(exp->recurrenceRule()=="weekly") gen=(utils::DateUtils::daysBetween(txn->date(),today)>=7);
        if(gen){
            auto r=std::make_unique<models::Expense>(utils::UUID::generate(),txn->amount(),txn->description(),
                txn->categoryId(),txn->accountId(),txn->userId(),today);
            r->setRecurring(true); r->setRecurrenceRule(exp->recurrenceRule()); r->setTags(txn->tags());
            txns_.push_back(std::move(r)); saveTransactions();
            notificationService_.notify(txn->userId(),models::NotificationType::RECURRING_PAYMENT,
                "Recurring: "+txn->description()+" ("+std::to_string(txn->amount())+")");
            ++cnt;
        }
    }
    return cnt;
}

// ══════════════════════════════════════════════════════════════════════
// ReportGenerator
// ══════════════════════════════════════════════════════════════════════

ReportGenerator::ReportGenerator(storage::StorageManager& s) : storage_(s) {}

ReportGenerator::Report ReportGenerator::buildReport(const std::vector<models::Transaction*>& txns, const std::string& label) const {
    Report r; r.periodLabel=label;
    std::map<std::string,double> catInc,catExp,accSum;
    double maxExp=0,totalExp=0; int expCnt=0;
    for(auto*t:txns){
        ++r.totalTransactions;
        if(t->transactionTypeName()=="Income"){r.totalIncome+=t->amount();catInc[t->categoryId()]+=t->amount();}
        else if(t->transactionTypeName()=="Expense"){r.totalExpense+=t->amount();catExp[t->categoryId()]+=t->amount();
            if(t->amount()>maxExp)maxExp=t->amount();totalExp+=t->amount();++expCnt;}
        accSum[t->accountId()]+=t->amount();
    }
    r.netSavings=r.totalIncome-r.totalExpense; r.highestExpense=maxExp;
    r.averageSpending=expCnt>0?totalExp/expCnt:0; r.accountSummary=accSum;
    std::vector<std::pair<std::string,double>> sc(catExp.begin(),catExp.end());
    std::sort(sc.begin(),sc.end(),[](const auto& a,const auto& b){return a.second>b.second;});
    size_t lim=std::min(sc.size(),size_t{5}); r.topCategories.assign(sc.begin(),sc.begin()+lim);
    r.categorySummary=catExp; for(auto&[c,a]:catInc)r.categorySummary[c]+=a;
    return r;
}

ReportGenerator::Report ReportGenerator::generateDaily(const std::string& userId) {
    std::string today=utils::DateUtils::today(); auto all=storage_.loadTransactions();
    std::vector<models::Transaction*> f; for(auto&t:all){if(t->userId()==userId&&t->date()==today)f.push_back(t.get());}
    return buildReport(f,"Daily - "+today);
}

ReportGenerator::Report ReportGenerator::generateWeekly(const std::string& userId) {
    std::tm tm={}; auto[y,m,d]=utils::DateUtils::extractYMD(utils::DateUtils::today());
    tm.tm_year=y-1900; tm.tm_mon=m-1; tm.tm_mday=d; std::mktime(&tm);
    int dow=tm.tm_wday; int sinceMon=(dow==0)?6:dow-1;
    auto mon=utils::DateUtils::parseDate(utils::DateUtils::today())-std::chrono::hours(24*sinceMon);
    auto sun=mon+std::chrono::hours(24*6);
    std::string ms=utils::DateUtils::formatDate(mon), ss=utils::DateUtils::formatDate(sun);
    auto all=storage_.loadTransactions();
    std::vector<models::Transaction*> f;
    for(auto&t:all){if(t->userId()==userId&&t->date()>=ms&&t->date()<=ss)f.push_back(t.get());}
    return buildReport(f,"Weekly - "+ms+" to "+ss);
}

ReportGenerator::Report ReportGenerator::generateMonthly(const std::string& userId, int year, int month) {
    std::string p=std::to_string(year)+"-"+(month<10?"0":"")+std::to_string(month);
    auto all=storage_.loadTransactions();
    std::vector<models::Transaction*> f;
    for(auto&t:all){if(t->userId()==userId&&t->date().substr(0,7)==p)f.push_back(t.get());}
    return buildReport(f,utils::DateUtils::monthName(month)+" "+std::to_string(year));
}

ReportGenerator::Report ReportGenerator::generateYearly(const std::string& userId, int year) {
    std::string ys=std::to_string(year); auto all=storage_.loadTransactions();
    std::vector<models::Transaction*> f;
    for(auto&t:all){if(t->userId()==userId&&t->date().substr(0,4)==ys)f.push_back(t.get());}
    return buildReport(f,"Yearly - "+ys);
}

ReportGenerator::Report ReportGenerator::generateForRange(const std::string& userId, const std::string& from, const std::string& to) {
    auto all=storage_.loadTransactions();
    std::vector<models::Transaction*> f;
    for(auto&t:all){if(t->userId()==userId&&t->date()>=from&&t->date()<=to)f.push_back(t.get());}
    return buildReport(f,from+" to "+to);
}

std::string ReportGenerator::exportToCsv(const Report& r) const {
    std::ostringstream csv;
    csv<<"Period,"<<r.periodLabel<<"\nTotal Income,"<<r.totalIncome<<"\nTotal Expense,"<<r.totalExpense
       <<"\nNet Savings,"<<r.netSavings<<"\nHighest Expense,"<<r.highestExpense
       <<"\nAverage Spending,"<<r.averageSpending<<"\nTotal Transactions,"<<r.totalTransactions<<"\n\nCategory,Amount\n";
    for(auto&[c,a]:r.categorySummary) csv<<c<<","<<a<<"\n";
    csv<<"\nAccount,Amount\n"; for(auto&[c,a]:r.accountSummary) csv<<c<<","<<a<<"\n";
    return csv.str();
}

std::string ReportGenerator::exportToJson(const Report& r) const {
    nlohmann::json j; j["period"]=r.periodLabel; j["total_income"]=r.totalIncome; j["total_expense"]=r.totalExpense;
    j["net_savings"]=r.netSavings; j["highest_expense"]=r.highestExpense; j["average_spending"]=r.averageSpending;
    j["total_transactions"]=r.totalTransactions;
    nlohmann::json c=nlohmann::json::object(); for(auto&[k,v]:r.categorySummary)c[k]=v; j["category_summary"]=c;
    nlohmann::json a=nlohmann::json::object(); for(auto&[k,v]:r.accountSummary)a[k]=v; j["account_summary"]=a;
    return j.dump(2);
}

std::string ReportGenerator::exportToTxt(const Report& r) const {
    std::ostringstream t;
    t<<"╔══════════════════════════════════════════╗\n║  "<<std::left<<std::setw(37)<<r.periodLabel<<"║\n╚══════════════════════════════════════════╝\n\n"
     <<"  Total Income:       "<<std::fixed<<std::setprecision(2)<<r.totalIncome
     <<"\n  Total Expense:      "<<r.totalExpense<<"\n  Net Savings:        "<<r.netSavings
     <<"\n  Highest Expense:    "<<r.highestExpense<<"\n  Average Spending:   "<<r.averageSpending
     <<"\n  Total Transactions: "<<r.totalTransactions<<"\n\n  ── Top Categories ──\n";
    for(auto&[c,a]:r.topCategories) t<<"    "<<c<<": "<<a<<"\n";
    return t.str();
}

void ReportGenerator::saveToFile(const std::string& content, const std::string& path) const {
    auto dir=std::filesystem::path(path).parent_path();
    if(!dir.empty()&&!std::filesystem::exists(dir)) std::filesystem::create_directories(dir);
    std::ofstream ofs(path); if(ofs.is_open()) ofs<<content;
}

// ══════════════════════════════════════════════════════════════════════
// SearchEngine
// ══════════════════════════════════════════════════════════════════════

SearchEngine::SearchEngine(storage::StorageManager& s) : storage_(s) {}

static bool containsIgnore(const std::string& hay, const std::string& needle) {
    if(needle.empty())return true;
    return std::search(hay.begin(),hay.end(),needle.begin(),needle.end(),
        [](char a,char b){return std::tolower(a)==std::tolower(b);})!=hay.end();
}

std::vector<models::Transaction*> SearchEngine::search(const std::string& userId, const Criteria& c) const {
    cache_ = storage_.loadTransactions();
    std::vector<models::Transaction*> r;
    for(auto& txn:cache_){
        if(txn->userId()!=userId)continue;
        if(c.dateFrom&&txn->date()<*c.dateFrom)continue;
        if(c.dateTo&&txn->date()>*c.dateTo)continue;
        if(c.amountMin&&txn->amount()<*c.amountMin)continue;
        if(c.amountMax&&txn->amount()>*c.amountMax)continue;
        if(c.categoryId&&txn->categoryId()!=*c.categoryId)continue;
        if(c.description&&!containsIgnore(txn->description(),*c.description))continue;
        if(c.tag){auto&tags=txn->tags();if(std::find(tags.begin(),tags.end(),*c.tag)==tags.end())continue;}
        if(c.accountId&&txn->accountId()!=*c.accountId)continue;
        if(c.transactionType&&txn->transactionTypeName()!=*c.transactionType)continue;
        r.push_back(txn.get());
    }
    return r;
}

}  // namespace finance::services
