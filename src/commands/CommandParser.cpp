#include "finance/commands/CommandParser.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace finance::commands {

void CommandParser::registerCommand(std::unique_ptr<ICommand> cmd)
{
    if (cmd) {
        commands_[cmd->name()] = std::move(cmd);
    }
}

bool CommandParser::execute(const std::string& input)
{
    auto tokens = tokenize(input);
    if (tokens.empty()) return true;

    std::string cmdName = tokens.front();
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    if (cmdName == "exit" || cmdName == "quit") {
        return false;
    }

    if (cmdName == "help") {
        printHelp();
        return true;
    }

    auto it = commands_.find(cmdName);
    if (it == commands_.end()) {
        std::cout << "Unknown command: " << cmdName << "\n"
                  << "Type 'help' to see available commands.\n";
        return true;
    }

    try {
        it->second->execute(args);
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }

    return true;
}

void CommandParser::printHelp() const
{
    std::cout << "\n╔══════════════════════════════════════════════╗\n"
              << "║          Available Commands                 ║\n"
              << "╚══════════════════════════════════════════════╝\n\n";

    // Find the longest command name for alignment.
    size_t maxLen = 0;
    for (const auto& [name, cmd] : commands_) {
        maxLen = std::max(maxLen, name.size());
    }

    for (const auto& [name, cmd] : commands_) {
        std::cout << "  " << name;
        size_t pad = maxLen - name.size() + 2;
        for (size_t i = 0; i < pad; ++i) std::cout << ' ';
        std::cout << cmd->description() << "\n";
    }
    std::cout << "  help";
    for (size_t i = 0; i < maxLen + 1; ++i) std::cout << ' ';
    std::cout << "Show this help message\n";
    std::cout << "  exit";
    for (size_t i = 0; i < maxLen + 1; ++i) std::cout << ' ';
    std::cout << "Exit the application\n";
    std::cout << std::endl;
}

std::vector<std::string> CommandParser::commandNames() const
{
    std::vector<std::string> names;
    for (const auto& [name, _] : commands_) {
        names.push_back(name);
    }
    return names;
}

std::vector<std::string> CommandParser::tokenize(const std::string& input)
{
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;

    for (char c : input) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

}  // namespace finance::commands
