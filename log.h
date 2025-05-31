#pragma once

#include "settings.h"

#include <iostream>
#include <string>
#include <cstdint>
#include <iomanip>

#ifdef __unix__
    #define COLOR_RESET "\033[0m"
    #define COLOR_CYAN "\033[36m"
    #define COLOR_GREEN "\033[32m"
    #define COLOR_RED "\033[31m"
#else
    #define COLOR_RESET
    #define COLOR_BLUE
    #define COLOR_GREEN
    #define COLOR_RED
#endif


class Log {
public:
    static Log& Instance() {
        static Log instance;
        return instance;
    }

    void info(std::string message) {
        std::cout << COLOR_CYAN << "INFO: " << COLOR_RESET << message << std::endl;
    }

    void debug(std::string message) {
        if(SETTINGS.showDebugOutput)
            std::cout << COLOR_GREEN << "DEBUG: " << COLOR_RESET << message << std::endl;
    }

    void err(std::string message) {
        std::cerr << COLOR_RED << "ERROR: " << COLOR_RESET << message << std::endl;
    }

    void fullErr(std::string message, std::string errorMessage) {
        std::cerr << COLOR_RED << "ERROR: " << COLOR_RESET << message << " " << errorMessage << std::endl;
    }

    void SDLerr(std::string message, const char* SDLMessage) {
        std::cerr << COLOR_RED << "SDL_ERROR: " << COLOR_RESET << message << " " << SDLMessage << std::endl;
    }

    void hex(std::string message, uint32_t hex) {
        std::cout << "HEX: " << message << " " << std::hex << std::setw(8) << std::setfill('0') << hex << std::dec << std::endl;
    }
};

#define LOG Log::Instance()