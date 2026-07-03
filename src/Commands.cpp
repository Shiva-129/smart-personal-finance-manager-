#include "finance/Commands.h"
#include "finance/Utils.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace finance::commands {

// ══════════════════════════════════════════════════════════════════════
// CommandParser
// ══════════════════════════════════════════════════════════════════════

void CommandParser::registerCommand(std::unique_ptr<ICommand> cmd) {
    if(cmd) commands_[cmd->name()]=std::move(cmd);
}

bool CommandParser::execute(const std::string& input) {
    auto tokens=tokenize(input); if(tokens.empty())return true;
    std::string name=tokens.front();
    std::vector<std::string> args(tokens.begin()+1,tokens.end());
    if(name=="exit"||name=="quit")return false;
    if(name=="help"){printHelp();return true;}
    auto it=commands_.find(name);
    if(it==commands_.end()){std::cout<<"Unknown: "<<name<<"\nType 'help'.\n";return true;}
    try{it->second->execute(args);}catch(const std::exception& e){std::cout<<"Error: "<<e.what()<<"\n";}
    return true;
}

void CommandParser::printHelp() const {
    std::cout<<"\n╔══════════════════════════════════════════╗\n║          Available Commands            ║\n╚══════════════════════════════════════════╝\n\n";
    size_t max=0; for(auto&[n,c]:commands_)max=std::max(max,n.size());
    for(auto&[n,c]:commands_){std::cout<<"  "<<n;for(size_t i=0;i<max-n.size()+2;++i)std::cout<<' ';std::cout<<c->description()<<"\n";}
    std::cout<<"  help";for(size_t i=0;i<max+1;++i)std::cout<<' ';std::cout<<"Show this help\n";
    std::cout<<"  exit";for(size_t i=0;i<max+1;++i)std::cout<<' ';std::cout<<"Exit\n";
    std::cout<<std::endl;
}

std::vector<std::string> CommandParser::commandNames() const {
    std::vector<std::string> n; for(auto&[k,v]:commands_)n.push_back(k); return n;
}

std::vector<std::string> CommandParser::tokenize(const std::string& input) {
    std::vector<std::string> t; std::string cur; bool q=false;
    for(char c:input){if(c=='"')q=!q;else if(c==' '&&!q){if(!cur.empty()){t.push_back(cur);cur.clear();}}else cur+=c;}
    if(!cur.empty())t.push_back(cur); return t;
}

}  // namespace finance::commands

// Command handlers and registration — in separate file to keep this manageable
