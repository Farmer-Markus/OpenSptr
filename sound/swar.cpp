#include "swar.h"
#include "swav.h"
#include "../byteutils.h"
#include "../filesystem.h"
#include "../log.h"

#include <fstream>

#define ID 0x0
#define FILESIZE 0x8
#define TOTAL_SAMPLES 0x38
#define OFFSET_SAMPLE_OFFSETS 0x3C

bool Swar::getHeader() {
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(dataOffset, std::ios::beg);
        
    // Read header ang get Information
    header.id = BYTEUTILS.getBytes(romStream, 4);

    if(header.id != 0x53574152) {
        LOG.hex("Failed to load swar, wrong header ID:", header.id);
        return false;
    }

    romStream.seekg(dataOffset + FILESIZE, std::ios::beg);
    header.fileSize = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(dataOffset + TOTAL_SAMPLES, std::ios::beg);
    header.totalSamples = BYTEUTILS.getLittleEndian(romStream, 4);

    for(uint32_t sample = 0; sample < header.totalSamples; sample++) {
        header.sampleOffsets.push_back(BYTEUTILS.getLittleEndian(romStream, 4));
    }

    return true;
}

bool Swar::getSound(Swav& swav, size_t sample) {
    std::ifstream& romStream = FILESYSTEM.getRomStream();

    size_t samples = header.sampleOffsets.size();
    // 0 Zählt auch schon als Eintrag!
    if(sample >= samples) {
        LOG.err("Swar::getSound: sample out of Range");
        return false;
    }

    swav.dataOffset = dataOffset + header.sampleOffsets[sample];
    // Wieder das selbe, 0 ist auch ein Eintrag
    swav.dataSize = (sample + 1 < samples) ? header.sampleOffsets[sample + 1] - header.sampleOffsets[sample] :
                                            header.fileSize - header.sampleOffsets[sample];
    return true;
}