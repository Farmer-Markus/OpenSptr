#pragma once

#include <cstdint>

#include "swav.h"
#include "../byteutils.h"


class Swar {
private:

public:
    struct InfoEntry {
        uint16_t fileID = 0; // 0x0 Fat file id
    } infoEntry;

    struct Header {
        uint32_t id = 0; // 0x0
        // uint16_t byteorder
        // uint16_t version
        uint32_t fileSize = 0; // 0x8
        // uint16_t headerSize
        // uint16_t totalBlocks (usually 1 = DATA)
        // uint32_t DataID = Data block
        // uint32_t filesize - 0x10
        // 0x20 reserved
        uint32_t totalSamples = 0; // Number of SWAV sample blocks 0x38
        std::vector<uint32_t> sampleOffsets; // Offets to sample blocks. offset angabe 4 bytes lang(uint32_t) und so viele offsets wie totalSamples(offsets vom start der swar datei aus!)
        // Sample blocks ... starting with Type (0=PCM8, 1=PCM16, 2=IMA-ADPCM)
    } header;

    uint32_t dataOffset = 0; // Relative to SDAT File offset!
    uint32_t dataSize = 0;

    /*struct Sound {
        // Relative to rom begin
        uint32_t offset = 0;
        uint32_t size = 0;
    };*/

    bool getHeader();

    // returns offset relative to rom Start. sample = der wievielte sound in der swar
    bool getSound(Swav& swav, uint16_t sample);

};