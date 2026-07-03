#ifndef FINANCE_UTILS_EXCEPTIONS_H
#define FINANCE_UTILS_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace finance::utils {

/// Base exception for all FinanceManager errors.
class FinanceException : public std::runtime_error {
public:
    explicit FinanceException(const std::string& message)
        : std::runtime_error(message) {}
};

/// Thrown when a user provides invalid input.
class InvalidInputException : public FinanceException {
public:
    explicit InvalidInputException(const std::string& details)
        : FinanceException("Invalid input: " + details) {}
};

/// Thrown when an operation would produce a negative balance.
class NegativeBalanceException : public FinanceException {
public:
    explicit NegativeBalanceException(const std::string& accountName)
        : FinanceException("Insufficient funds in account: " + accountName) {}
};

/// Thrown when the requested entity is not found.
class NotFoundException : public FinanceException {
public:
    explicit NotFoundException(const std::string& entityType,
                                const std::string& id = "")
        : FinanceException("Not found: " + entityType +
                           (id.empty() ? "" : " [" + id + "]")) {}
};

/// Thrown when a duplicate entity is being created.
class DuplicateException : public FinanceException {
public:
    explicit DuplicateException(const std::string& entityType,
                                 const std::string& name)
        : FinanceException("Duplicate " + entityType + ": " + name) {}
};

/// Thrown when authentication fails.
class AuthenticationException : public FinanceException {
public:
    explicit AuthenticationException(const std::string& details)
        : FinanceException("Authentication failed: " + details) {}
};

/// Thrown when a credit card limit would be exceeded.
class CreditLimitException : public FinanceException {
public:
    explicit CreditLimitException(const std::string& accountName)
        : FinanceException("Credit limit exceeded for: " + accountName) {}
};

/// Thrown on file I/O errors.
class FileIOException : public FinanceException {
public:
    explicit FileIOException(const std::string& path)
        : FinanceException("File I/O error: " + path) {}
};

/// Thrown when JSON data is corrupted.
class CorruptedDataException : public FinanceException {
public:
    explicit CorruptedDataException(const std::string& details)
        : FinanceException("Corrupted data: " + details) {}
};

}  // namespace finance::utils

#endif  // FINANCE_UTILS_EXCEPTIONS_H
