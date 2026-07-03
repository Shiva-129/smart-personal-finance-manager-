#ifndef FINANCE_COMMANDS_COMMANDPARSER_H
#define FINANCE_COMMANDS_COMMANDPARSER_H

#include "finance/commands/ICommand.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace finance::commands {

/**
 * @brief Tokenises user input and dispatches to the matching ICommand.
 *
 * Uses the Command pattern: each action is encapsulated in its own
 * command object, registered here with a keyword.
 */
class CommandParser {
public:
    /// Register a command so it can be invoked by its name.
    void registerCommand(std::unique_ptr<ICommand> cmd);

    /// Parse a single line of input and execute the matching command.
    /// Returns false when the "exit" command is received.
    bool execute(const std::string& input);

    /// Print the help listing (all registered commands).
    void printHelp() const;

    /// Return the list of registered command names.
    std::vector<std::string> commandNames() const;

private:
    /// Split a line into tokens (handles double-quoted strings).
    static std::vector<std::string> tokenize(const std::string& input);

    std::map<std::string, std::unique_ptr<ICommand>> commands_;
};

}  // namespace finance::commands

#endif  // FINANCE_COMMANDS_COMMANDPARSER_H
