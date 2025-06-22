#include <fstream>


#include <cstring>

#include "swav.h"
#include "pcm.h"
#include "../filesystem.h"
#include "../byteutils.h"
#include "../log.h"


// Sample header offsets
#define SMPL_TYPE 0x0
#define SMPL_LOOP 0x1
#define SMPL_SAMPLERATE 0x2
#define SMPL_LOOP_OFFSET 0x6
#define SMPL_SND_LENGTH 0x8

#define HEADER_SIZE 0xC


bool Swav::getHeader(sndType::Swav& swav) { // Not used
    sndType::Swav::Header& header = swav.header;
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(swav.dataOffset, std::ios::beg);
    
    return true;
}

bool Swav::getSampleHeader(sndType::Swav& swav) {
    sndType::Swav::SampleHeader& header = swav.sampleHeader;
    std::ifstream& romStream = FILESYSTEM.getRomStream();

    romStream.seekg(swav.dataOffset, std::ios::beg);
    header.type = romStream.get();

    romStream.seekg(swav.dataOffset + SMPL_LOOP, std::ios::beg);
    header.loop = romStream.get();

    romStream.seekg(swav.dataOffset + SMPL_SAMPLERATE, std::ios::beg);
    header.samplingRate = BYTEUTILS.getLittleEndian(romStream, 2);
    
    romStream.seekg(swav.dataOffset + SMPL_LOOP_OFFSET, std::ios::beg);
    header.loopOffset = BYTEUTILS.getLittleEndian(romStream, 2);

    romStream.seekg(swav.dataOffset + SMPL_SND_LENGTH, std::ios::beg);
    header.nonLoopLength = BYTEUTILS.getLittleEndian(romStream, 4);
    // Nur bis zum Anfang vom loop bereich und nicht bis ende von daten!!!!!!

    /*// Glaube nicht das das stimmt !
    if(header.type == 2) { // ima-adpcm uses blocks
        header.totalBlocks = (swav.dataSize - HEADER_SIZE) / 36; // Frag mich nicht woher ich die 36 habe... (32 oder 36??)
    }*/ // EIN BLOCK... NUR EINEN FUCKING BLOCK HABEN SWAV'S IN SWAR ARCHIVEN!!!

    return true;
}


bool Swav::convert(sndType::Swav& swav, std::vector<uint8_t>& outBuffer) {
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    
    if(swav.header.filesize > 0) { // Normal swav file // not used
        sndType::Swav::Header& header = swav.header;

    } else if(swav.sampleHeader.nonLoopLength > 0) { // Swar swav sample
        sndType::Swav::SampleHeader& header = swav.sampleHeader;

        romStream.seekg(swav.dataOffset + HEADER_SIZE, std::ios::beg);
        std::vector<uint8_t> buffer(swav.dataSize);
        romStream.read((char*)buffer.data(), swav.dataSize - HEADER_SIZE);

        // Data size * (2 samples per byte) * (stereo)
        std::vector<int16_t> pcmMonoData((swav.dataSize - HEADER_SIZE -4) * 2 + 1);
        //std::vector<int16_t> pcmData(pcmMonoData.size() * 2);

        //                                               | Hat KEINEN Blockheader
        PCM.decodeImaAdpcm(buffer, pcmMonoData, 1, 0, 0, false);

        // Sounds sind mono aber sdl ist stereo also müssen beide Seiten(l,r) die gleichen sounds haben
        std::vector<int16_t> pcmData(pcmMonoData.size() * 2);
        for(size_t i = 0; i < pcmMonoData.size(); i++) {
            pcmData[2 * i] = pcmMonoData[i];
            pcmData[2 * i + 1] = pcmMonoData[i];
        }

        size_t outBufferSize = outBuffer.size();
        size_t pcmDataSize = pcmData.size();
        outBuffer.resize(outBufferSize + pcmDataSize * sizeof(int16_t));
        std::memcpy(outBuffer.data() + outBufferSize, pcmData.data(), pcmDataSize * sizeof(int16_t));

    } else {
        LOG.err("Swav::convert: Header not filled!");
        return false;
    }
    return true;
}