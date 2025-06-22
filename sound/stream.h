#pragma once

#include <cstdint>

#include "sdat.h"
#include "types.h"
#include "audioId.h"
#include "sound.h"

class Stream {
private:
    struct WAVHeader {
        char riff[4] = {'R','I','F','F'};
        uint32_t chunkSize;
        char wave[4] = {'W','A','V','E'};
        char fmt[4] = {'f','m','t',' '};
        uint32_t subchunk1Size = 16;
        uint16_t audioFormat = 1;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample = 16;
        char data[4] = {'d','a','t','a'};
        uint32_t dataSize;
    };


public:
    static Stream& Instance() {
        static Stream instance;
        return instance;
    }

    // Writes STRM header values into STRM pointer
    bool getHeader(sndType::Strm& strm);
    
    // Converts STRM into wave(wav)
    bool convert(sndType::Strm strm, std::vector<uint8_t>& sound);

    // Updates STRM buffer for audio mixer. Decodes only 1 block!
    bool updateBuffer(Soundsystem::StrmSound& sound, int len, uint16_t targetSampleRate);
};

#define STREAM Stream::Instance()