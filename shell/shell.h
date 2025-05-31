#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <signal.h>
#include <sstream>
#include <filesystem>
#include "../filesystem.h"

#include "command.h"


#ifdef __unix__
    #define COLOR_RESET "\033[0m"
    #define COLOR_BLUE "\033[34m"
    #define COLOR_GREEN "\033[32m"
#else
    #define COLOR_RESET
    #define COLOR_BLUE
    #define COLOR_GREEN
#endif



class Shell {
private:
    Command command;

    std::filesystem::path currPath = "";

    bool inputHandling() {
        std::string cmd;
        std::getline(std::cin, cmd);

        std::istringstream iss(cmd);
        std::vector<std::string> args;
        std::string arg;
        while(iss >> arg) {
            args.push_back(arg);
        }
        
        if(cmd.empty()) {
            return true;
        }

        if(cmd == "clear") {
            command.clear();
            return true;

        } else if(args[0] == "dir" || args[0] == "ls") {
            std::filesystem::path path = currPath;
            if(args.size() > 1)
                path /= args[1];
            command.list(path);
            return true;

        } else if(cmd == "exit") {
            return false;

        } else if(args[0] == "cd") {
            if(args.size() > 1) {
                command.changeDirectory(currPath, args[1]);
            } else {
                command.changeDirectory(currPath, "");
            }
        } else if(args[0] == "help") {
            if(args.size() > 1) {
                command.help(args[1]);
            } else {
                command.help("");
            }
        } else if(args[0] == "extract") {
            if(args.size() > 2) {
                command.extract(currPath / args[1], args[2]);
            } else if(args.size() > 1) {
                command.extract(currPath / args[1], "");
            } else {
                command.extract("", "");
            }
        }
        
        
        else {
            std::cout << "Command not found. Try 'help' to view available commands." << std::endl;
            return true;
        }

        return true;
    }


public:
    bool enter() {
        std::cout << "This is an interactive shell. Type 'help' to get an overview over the available commands." << std::endl;
        
        while(true) {
            std::cout << COLOR_GREEN << "rom:/" << currPath.string() << COLOR_BLUE << " > " << COLOR_RESET;
            if(!inputHandling())
                return true;
        }
    }
};