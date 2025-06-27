#pragma once

#include <cstdint>

#include "sound.h"

class Strm {
private:
    /*struct WAVHeader {
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
    };*/


public:

    struct InfoEntry {
        uint16_t fileID = 0; // 0x0
        // uint16_t unknown 0x2
        uint8_t vol = 0; // 0x4 volume
        uint8_t pri = 0; // 0x5 priority? maybe?
        uint8_t ply = 0; // 0x6 play?
        // 5 Bytes reserved
    } infoEntry;

    struct Header {
        uint32_t id = 0;
        uint32_t filesize = 0;
        uint8_t type = 0;
        uint8_t loop = 0;
        uint8_t channels = 0;
        uint16_t samplingRate = 0;
        uint32_t loopOffset = 0; // in Samples (decoded audio!)
        uint32_t totalSamples = 0; // (decoded audio!)
        uint32_t waveDataOffset = 0;
        uint32_t totalBlocks = 0;
        uint32_t blockLength = 0;
        uint32_t samplesBlock = 0; // Samples per block (decoded audio!)
        uint32_t lastBlockLength = 0;
        uint32_t samplesLastBlock = 0; // (decoded audio!)
        uint32_t dataSize = 0;
        //uint32_t dataOffset;
    } header;

    uint32_t dataOffset = 0; // Relative to SDAT File offset! 
    uint32_t dataSize = 0;

    std::vector<uint8_t> rawData; // Used for audio playback

    // Writes STRM header values into STRM pointer
    bool getHeader();
    
    // Converts STRM into wave(wav)
    //bool convert(sndType::Strm strm, std::vector<uint8_t>& sound);

    // Updates STRM buffer for audio mixer. Decodes only 1 block!
    bool updateBuffer(Soundsystem::StrmSound& sound, int len, uint16_t targetSampleRate);
};