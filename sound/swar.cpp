#include "swar.h"
#include "types.h"
#include "../byteutils.h"
#include "../filesystem.h"
#include "../log.h"

#include <fstream>

#define ID 0x0
#define FILESIZE 0x8
#define TOTAL_SAMPLES 0x38
#define OFFSET_SAMPLE_OFFSETS 0x3C

bool Swar::getHeader(sndType::Swar& swar) {
    sndType::Swar::Header& header = swar.header;
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(swar.dataOffset, std::ios::beg);
        
    // Read header ang get Information
    header.id = BYTEUTILS.getBytes(romStream, 4);

    if(header.id != 0x53574152) {
        LOG.hex("Failed to load swar, wrong header ID:", header.id);
        return false;
    }

    romStream.seekg(swar.dataOffset + FILESIZE, std::ios::beg);
    header.fileSize = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(swar.dataOffset + TOTAL_SAMPLES, std::ios::beg);
    header.totalSamples = BYTEUTILS.getLittleEndian(romStream, 4);

    for(uint32_t sample = 0; sample < header.totalSamples; sample++) {
        header.sampleOffsets.push_back(BYTEUTILS.getLittleEndian(romStream, 4));
    }

    return true;
}

Swar::Sound Swar::getSound(sndType::Swar& swar, size_t sample) {
    sndType::Swar::Header& header = swar.header;
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    Sound sound;

    size_t samples = header.sampleOffsets.size();
    // 0 Zählt auch schon als Eintrag!
    if(sample >= samples) {
        LOG.err("Swar::getSound: sample out of Range");
        return sound;
    }

    sound.offset = swar.dataOffset + header.sampleOffsets[sample];
    // Wieder das selbe, 0 ist auch ein Eintrag
    sound.size = (sample + 1 < samples) ? header.sampleOffsets[sample + 1] - sound.offset :
                                            header.fileSize - sound.offset;

    return sound;
}