#pragma once

#include <cstdint>

#include "sdat.h"
#include "types.h"
#include "audioId.h"

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

    // Decode IMAADPCM BLock
    void decodeBlock(const std::vector<uint8_t>& blockData, std::vector<int16_t>& pcmData);


public:
    struct StrmHeader {
        uint32_t Id = 0;
        uint32_t filesize = 0;
        uint8_t type = 0;
        uint8_t loop = 0;
        uint8_t channels = 0;
        uint16_t samplingRate = 0;
        uint32_t loopOffset = 0;
        uint32_t totalSamples = 0;
        uint32_t waveDataOffset = 0;
        uint32_t totalBlocks = 0;
        uint32_t blockLength = 0;
        uint32_t samplesBlock = 0; // Samples per block
        uint32_t lastBlockLength = 0;
        uint32_t samplesLastBlock = 0;
        uint32_t dataSize = 0;
        //uint32_t dataOffset;
    };

    static Stream& Instance() {
        static Stream instance;
        return instance;
    }

    // Writes STRM header values into given pointer
    bool getHeader(STRM strm, StrmHeader& header);
    
    // Converts STRM into wave(wav)
    bool convert(STRM strm, std::vector<uint8_t>& sound);
};

#define STREAM Stream::Instance()