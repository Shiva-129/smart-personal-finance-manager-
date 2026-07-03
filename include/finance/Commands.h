#ifndef FINANCE_COMMANDS_H
#define FINANCE_COMMANDS_H

#include "FinanceManager.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace finance::commands {

// ══════════════════════════════════════════════════════════════════════
// ICommand — abstract interface
// ══════════════════════════════════════════════════════════════════════

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
    virtual std::string usage() const = 0;
    virtual void execute(const std::vector<std::string>& args) = 0;
};

// ══════════════════════════════════════════════════════════════════════
// CommandParser — tokenizer + dispatcher
// ══════════════════════════════════════════════════════════════════════

class CommandParser {
public:
    void registerCommand(std::unique_ptr<ICommand> cmd);
    bool execute(const std::string& input);
    void printHelp() const;
    std::vector<std::string> commandNames() const;
private:
    static std::vector<std::string> tokenize(const std::string& input);
    std::map<std::string, std::unique_ptr<ICommand>> commands_;
};

}  // namespace finance::commands

#endif  // FINANCE_COMMANDS_H
