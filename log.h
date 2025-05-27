#pragma once

#include "settings.h"

#include <iostream>
#include <string>
#include <cstdint>
#include <iomanip>


class Log {
public:
    static Log& Instance() {
        static Log instance;
        return instance;
    }

    void info(std::string message) {
        std::cout << "INFO: " << message << std::endl;
    }

    void debug(std::string message) {
        if(SETTINGS.showDebugOutput)
            std::cout << "DEBUG: " << message << std::endl;
    }

    void err(std::string message) {
        std::cerr << "ERROR: " << message << std::endl;
    }

    void fullErr(std::string message, std::string errorMessage) {
        std::cerr << "ERROR:" << message << " " << errorMessage << std::endl;
    }

    void SDLerr(std::string message, const char* SDLMessage) {
        std::cerr << "SDL_ERROR:" << message << " " << SDLMessage << std::endl;
    }

    void hex(std::string message, uint32_t hex) {
        std::cout << "HEX:" << message << " " << std::hex << std::setw(8) << std::setfill('0') << hex << std::dec << std::endl;
    }
};

#define LOG Log::Instance()