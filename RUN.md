# RUN — Smart Personal Finance & Expense Manager

## Prerequisites

```bash
# Build the project
cmake -B build -S . -DBUILD_TESTS=ON && cmake --build build

# Run unit tests
./build/tests/finance_tests

# Run the CLI
./build/finance_manager
```

---

## IDE Agent: Test the CLI (copy the block below)

```
# Context
You are testing a C++17 CLI personal finance manager. The project is built with CMake.
The binary at `./build/finance_manager` implements a REPL with ~25 commands.

# Setup
1. Clean any previous data: `rm -rf data/`
2. Build: `cmake -B build -S . -DBUILD_TESTS=ON && cmake --build build`
3. Run: `./build/finance_manager`
4. Use `help` to list all commands.

# Full Workflow to Execute

## 1. Register
   >> register alice mypassword Alice
   Expect: "User 'alice' registered."

## 2. Login
   >> login alice mypassword
   Expect: "Welcome, Alice!" (prompt changes to "alice> ")

## 3. Add Accounts
   >> add-account Cash Wallet INR
   >> add-account Savings SavingsAccount INR
   >> add-account "Credit Card" PlatinumCard INR
   >> add-account Wallet PocketMoney INR
   Expect each: "Account '...' (Cash|Savings|Credit Card|Wallet) created."
   Note: The account ID is printed — copy it for later steps.

## 4. List Accounts
   >> list-accounts
   Expect: Table showing all 4 accounts with balance 0.00 INR

## 5. Add Income
   # Use the account IDs from step 3. Category IDs are auto-generated from defaults.
   # Fallback: use any string as category ID (system will accept it).
   >> add-income 60000 "Monthly salary" salary <WALLET_ACCOUNT_ID>
   >> add-income 5000 "Freelance project" freelance <WALLET_ACCOUNT_ID>
   Expect each: "Income recorded."

## 6. Add Expenses
   >> add-expense 15000 "Rent payment" rent <WALLET_ACCOUNT_ID>
   >> add-expense 2000 "Groceries" food <WALLET_ACCOUNT_ID>
   >> add-expense 500 "Bus pass" travel <WALLET_ACCOUNT_ID>
   Expect each: "Expense recorded."

## 7. Transfer Between Accounts
   >> transfer 10000 "Move to savings" <FROM_ACCOUNT_ID> <TO_ACCOUNT_ID>
   Expect: "Transfer done."

## 8. List Transactions
   >> list-transactions
   Expect: Table with all 5-6 transactions showing date, type, amount, description.

## 9. List Accounts (verify balances changed)
   >> list-accounts
   Expect: Updated balances reflecting income, expenses, and transfer.

## 10. Set Budget
    >> set-budget food 5000
    Expect: "Budget set."

## 11. Show Budget
    >> show-budget
    Expect: Shows budget for Food category with limit and spent amount.

## 12. Create Goal
    >> create-goal "Emergency Fund" 100000 2026-12-31
    Expect: "Goal 'Emergency Fund' created. ID: ..."

## 13. Contribute to Goal
    >> contribute-goal <GOAL_ID> 5000
    Expect: "Contributed."

## 14. Show Goals
    >> show-goals
    Expect: Shows goal with progress percentage.

## 15. Search Transactions
    >> search * * 5000 *
    Expect: List of transactions with amount >= 5000.

## 16. Generate Report
    >> report monthly
    Expect: Text report with total income, expenses, net savings, top categories.

## 17. Export Report
    >> export csv monthly /tmp/report.csv
    Expect: "Exported to /tmp/report.csv"
    Then verify: `cat /tmp/report.csv` shows CSV data.

## 18. Check Notifications
    >> notifications
    Expect: Shows any budget/goal notifications.

## 19. Delete + Undo
    >> list-transactions (copy one transaction ID)
    >> delete-transaction <TXN_ID>
    Expect: "Deleted. Use 'undo' to restore."
    >> undo
    Expect: "Undone."
    >> list-transactions
    Expect: The deleted transaction is back.

## 20. Process Recurring
    # First add a recurring expense:
    >> add-expense 999 "Netflix subscription" entertainment <WALLET_ACCOUNT_ID>
    Then create recurring entries:
    >> process-recurring
    Expect: "Generated X recurring transaction(s)."

## 21. Change Password
    >> change-password mypassword newpass123
    Expect: "Password changed."
    >> logout
    >> login alice newpass123
    Expect: "Welcome, Alice!"

## 22. Error Handling
    >> register
    Expect: "Usage: register <username> <password> [display-name]"
    >> nonexistent-command
    Expect: "Unknown: nonexistent-command"
    >> add-income -100 "Invalid" test test test
    Expect: "Error: Invalid input: Amount must be positive"

## 23. Freeze and Close Account
    >> list-accounts (copy an account ID)
    >> freeze-account <ACCOUNT_ID>
    >> close-account <ACCOUNT_ID>
    Expect each: "Frozen." / "Closed."

## 24. Exit
    >> exit
    Expect: "Goodbye!"

# Verification
- Re-run the app and login again — all data should persist from disk.
- Run `./build/tests/finance_tests` — all 93+ tests must pass.
