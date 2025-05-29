#pragma once

#include "types.h"
#include "../byteutils.h"

#include <cstdint>

class Swar {
private:

public:
    static Swar& Instance() {
        static Swar instance;
        return instance;
    }

    struct Sound {
        // Relative to rom begin
        uint32_t offset = 0;
        uint32_t size = 0;
    };

    bool getHeader(sndType::Swar& swar);

    // returns offset relative to rom Start. sample = der wievielte sound in der swar
    Sound getSound(sndType::Swar& swar, size_t sample);

};

#define SWAR Swar::Instance()