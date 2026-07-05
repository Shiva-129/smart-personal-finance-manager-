# Smart Personal Finance & Expense Manager 

Managing money shouldn't be complicated. This project is a lightweight, terminal-based personal finance assistant built in modern C++17. It helps you keep track of your cash, savings, and credit cards, set budgets, and plan for your savings goals—all from a fast, local command line interface.

## Why You'll Love It

* **Multiple Accounts, One Place:** Manage Cash, Savings, Current, Credit Cards, and Wallets with ease.
* **Smart Budgets:** Set spending limits for different categories. The app warns you when you reach 80% of your limit and alerts you if you go over.
* **Savings Goals:** Keep track of your dreams by creating goals and contributing to them.
* **Recurring Transactions:** Automate your regular bills and income.
* **Advanced Search & Reports:** Filter your transactions with precision and export them to CSV, JSON, or TXT.
* **Local & Thread-Safe:** Your data stays private on your machine in simple JSON files, protected by thread-safe operations.

## How It's Built

The application is structured using a clean, layered architecture:

* **Presentation Layer (CLI / REPL):** An interactive prompt that tokenizes and routes your commands.
* **Service Layer:** The core business logic orchestrating accounts, transactions, budgets, and reporting.
* **Repository Layer:** Thread-safe CRUD operations storing data in local JSON files.
* **Domain Models:** Clean polymorphic classes representing various account and transaction types.

## Project Structure

* **`include/` & `src/`:** The source code and header files containing the application logic.
* **`tests/`:** A comprehensive test suite powered by GoogleTest.
* **`data/`:** The automatically generated directory for local database storage.

---

Looking for details on compilation, installation, or how to start the app? Check out [RUN.md](RUN.md).
