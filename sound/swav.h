#pragma once

#include <cstdint>

#include "types.h"

class Swav {
private:

public:
    static Swav& Instance() {
        static Swav instance;
        return instance;
    }


    bool getHeader(sndType::Swav& swav);
    
    // Used by SWAR note extraction
    bool getSampleHeader(sndType::Swav& swav);

    bool convert(sndType::Swav& swav, std::vector<uint8_t>& sound, uint16_t targetSampleRate);
};

#define SWAV Swav::Instance()