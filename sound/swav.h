#pragma once

#include <cstdint>
#include <vector>


class Swav {
private:

public:

    struct Header { // not used
        uint32_t id = 0;
        // uint16_t byteorder
        // uint16_t version
        uint32_t filesize = 0;
        // uint16_t headersize
        uint16_t totalBlocks = 0;
        // uint32_t id DATA block
        // uint32_t filesize -10
        // Sample blocks ...
    } header;

    struct SampleHeader {
        uint8_t type = 0;
        uint8_t loop = 0;
        uint16_t samplingRate = 0;
        // uint16_t time
        uint16_t loopOffset = 0;
        uint32_t nonLoopLength = 0; // *4 nehmen um byte länge zu bekommen
        // Data ... samples (with 32bit header in case of ADPCM)

        //uint32_t totalBlocks = 0; // ONLY ONE BLOCK!
    } sampleHeader;

    // Raw pcm data for playback
    // Not pitched but decoded
    std::vector<int16_t> soundData;
    uint32_t dataOffset = 0;
    uint32_t dataSize = 0;

    // For standalone SWAV files
    bool getHeader();
    
    // For SWAR notes
    bool getSampleHeader();

    bool read();

    // @param targetSampleRate: Leave 0 or >0 to keep samplerate
    bool convert(std::vector<uint8_t>& sound, uint16_t targetSampleRate,
                int8_t semitonePitch);
};