# Running the Smart Personal Finance & Expense Manager

This guide provides step-by-step instructions to compile, test, and run the C++17 CLI Personal Finance Manager, followed by a concrete walkthrough example showing how to use the interactive application.

---

## 📋 Prerequisites

Before building the project, ensure your system has the following installed:
- **C++17 Compiler** (GCC 9+, Clang 10+, or MSVC 2019+)
- **CMake** (Version 3.16 or higher)
- **Make** or another build system generator
- **Internet Connection** (to download `nlohmann_json` and `googletest` via CMake's FetchContent)

---

## 🛠️ Step 1: Building the Project

Follow these steps to configure and build the binaries:

1. **Configure CMake:**
   Initialize the build directory and configure compilation options. To include unit tests, enable `BUILD_TESTS`.
   ```bash
   cmake -B build -S . -DBUILD_TESTS=ON
   ```

2. **Build the Code:**
   Compile the library, CLI executable, and tests:
   ```bash
   cmake --build build
   ```

Upon a successful build, the primary executables are generated under the `build` directory:
- **CLI Application:** `./build/src/finance_manager`
- **Unit Test Runner:** `./build/tests/finance_tests`

---

## 🧪 Step 2: Running Unit Tests

Verify that your build is correct and all internal components work as expected by running the tests:

```bash
./build/tests/finance_tests
```

> [!NOTE]
> There are 93 unit tests validating auth logic, accounts, budgets, goals, transactions, reporting, and storage. All of them should report `[  PASSED  ]`.

---

## 🚀 Step 3: Run the CLI Application

Start the interactive console-based Personal Finance Manager:

```bash
./build/src/finance_manager
```

On startup, you will see a welcome header and an interactive command prompt `> `:

```text
2026-07-04T20:34:32 [INFO] Storage initialized at: data
2026-07-04T20:34:32 [INFO] Finance Manager initializing...
2026-07-04T20:34:32 [INFO] Finance Manager ready.

  ╔══════════════════════════════════════════╗
  ║   Smart Personal Finance & Expense Mgr   ║
  ║           Version 1.0.0                  ║
  ╚══════════════════════════════════════════╝
  Type 'help' for commands.

> 
```

---

## 💡 Command-Line Example Walkthrough

Here is a complete, realistic scenario demonstrating how to register, log in, create accounts, record transactions, and generate reports.

### 🔑 1. Create a User and Log In
First, register a new account and then log into the session:

```text
> register alice mypassword Alice
User 'alice' registered.

> login alice mypassword
Welcome, Alice!
alice> 
```
> [!TIP]
> Once logged in, your prompt changes from `>` to `alice>`, indicating you are authenticated.

### 💳 2. Setup Your Accounts
Create two different accounts: a cash wallet and a savings account:

```text
alice> add-account Cash Wallet INR
Account 'Wallet' (Cash) created.

alice> add-account Savings SavingsAccount INR
Account 'SavingsAccount' (Savings) created.
```

### 🔍 How to Find Account UUIDs
To transfer funds or record transactions, the app requires UUIDs for references. Since the CLI tables show display names, retrieve the system-generated UUIDs by opening the local JSON database file:
```bash
cat data/accounts.json
```
For example, you will see:
```json
[
  {
    "id": "77c54456-2d6e-4be2-a5dd-1550ed08b44b",
    "name": "Wallet",
    "type": "Cash"
  },
  {
    "id": "a1b2c3d4-e5f6-7a8b-9c0d-1e2f3a4b5c6d",
    "name": "SavingsAccount",
    "type": "Savings"
  }
]
```
*(Copy these UUIDs for use in the subsequent steps)*

---

### 💵 3. Record Transactions
Record your income and expenses. The CLI commands take arguments in this structure:
- `add-income <amount> <description> <category-id> <account-uuid>`
- `add-expense <amount> <description> <category-id> <account-uuid>`

#### Record Income:
```text
alice> add-income 50000 "Monthly Salary" salary 77c54456-2d6e-4be2-a5dd-1550ed08b44b
Income recorded.
```

#### Record Expense:
```text
alice> add-expense 1500 "Groceries purchase" food 77c54456-2d6e-4be2-a5dd-1550ed08b44b
Expense recorded.
```

#### Check Updated Balances:
```text
alice> list-accounts

── Accounts ──
  Wallet (Cash) Balance: 48500.00 INR
  SavingsAccount (Savings) Balance: 0.00 INR
```

---

### 🔄 4. Transfer Funds Between Accounts
Move funds from your wallet to savings:
`transfer <amount> <description> <from-account-uuid> <to-account-uuid>`

```text
alice> transfer 10000 "Savings deposit" 77c54456-2d6e-4be2-a5dd-1550ed08b44b a1b2c3d4-e5f6-7a8b-9c0d-1e2f3a4b5c6d
Transfer done.
```

Verify the transfer worked:
```text
alice> list-accounts

── Accounts ──
  Wallet (Cash) Balance: 38500.00 INR
  SavingsAccount (Savings) Balance: 10000.00 INR
```

---

### 🎯 5. Set up a Budget & Savings Goal

#### Create a Monthly Budget:
Set a limit of `5000 INR` on the `food` category:
```text
alice> set-budget food 5000
Budget set.
```

#### Track Savings Goals:
Create a goal for an emergency fund of `100000 INR` with a deadline of `2026-12-31`:
```text
alice> create-goal "Emergency Fund" 100000 2026-12-31
Goal 'Emergency Fund' created. ID: 88f72c3d-be6d-4952-ba61-893f412dcfc9
```
*(Contribute `5000 INR` towards this savings goal)*
```text
alice> contribute-goal 88f72c3d-be6d-4952-ba61-893f412dcfc9 5000
Contributed.

alice> show-goals

── Goals ──
  Emergency Fund Target:100000.00 Current:5000.00 5%
```

---

### 📊 6. Generate Reports & Export Data

#### Print a Monthly Summary Report:
```text
alice> report monthly

╔══════════════════════════════════════════╗
║  July 2026                               ║
╚══════════════════════════════════════════╝

  Total Income:       50000.00
  Total Expense:      1500.00
  Net Savings:        48500.00
  Highest Expense:    1500.00
  Average Spending:   1500.00
  Total Transactions: 2

  ── Top Categories ──
    food: 1500.00
```

#### Export Data to CSV:
`export <csv|json|txt> <period> <output-file-path>`
```text
alice> export csv monthly report.csv
Exported to report.csv
```
This saves a detailed CSV report file to your project root.

---

### 🔄 7. Delete & Undo Transactions
If you make a mistake, you can delete a transaction and restore it using `undo`:

#### List transactions to see recent activity:
```text
alice> list-transactions

── Transactions ──
  [2026-07-04] Income   50000.00 Monthly Salary
  [2026-07-04] Expense  1500.00  Groceries purchase
  [2026-07-04] Transfer 10000.00 Savings deposit
```

#### Retrieve the Transaction ID:
To edit or delete a transaction, locate its ID in the local JSON database:
```bash
cat data/transactions.json
```
*(Copy the target transaction's `"id"` field)*

#### Delete and Undo:
```text
alice> delete-transaction 4e60beba-d185-433d-94f8-ac8b14fb90b1
Deleted. Use 'undo' to restore.

alice> undo
Undone.
```

---

### 🚪 8. Exit
When you are done, log out and close the program:

```text
alice> logout
Logged out.

> exit
Goodbye!
```

---

## 💾 Storage & Data Persistence

All your accounts, budgets, goals, and transactions are stored locally inside the `data/` directory in JSON files:
- `data/users.json` — User credentials and account details.
- `data/accounts.json` — Balances, account types, and currencies.
- `data/transactions.json` — Logs of incomes, expenses, and transfers.
- `data/budgets.json` — Category budget limits and spending totals.
- `data/goals.json` — Savings goal targets, contributions, and deadlines.
- `data/notifications.json` — Alerts (e.g., budget exceeded or warnings).

These files are read automatically every time you start the app, so your progress is fully persisted.
