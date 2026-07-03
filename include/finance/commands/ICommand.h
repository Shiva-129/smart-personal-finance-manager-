#ifndef FINANCE_COMMANDS_ICOMMAND_H
#define FINANCE_COMMANDS_ICOMMAND_H

#include <string>
#include <vector>

namespace finance::commands {

/**
 * @brief Abstract interface for a user-invokable command.
 *
 * Each concrete command wraps a single action (login, add-account,
 * add-income, etc.) and is registered with the CommandParser.
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /// The keyword that triggers this command (e.g. "login", "add-income").
    virtual std::string name() const = 0;

    /// A one-line description shown in the help listing.
    virtual std::string description() const = 0;

    /// Usage string, e.g. "login <username> <password>".
    virtual std::string usage() const = 0;

    /// Execute the command with the given arguments (excluding the name).
    virtual void execute(const std::vector<std::string>& args) = 0;
};

}  // namespace finance::commands

#endif  // FINANCE_COMMANDS_ICOMMAND_H
