#include <fstream>


#include <cstring>
#include <vector>

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


bool Swav::getHeader() { // Not used
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(dataOffset, std::ios::beg);
    
    return true;
}

bool Swav::getSampleHeader() {
    SampleHeader* header = &sampleHeader;
    std::ifstream& romStream = FILESYSTEM.getRomStream();

    romStream.seekg(dataOffset, std::ios::beg);
    header->type = romStream.get();

    romStream.seekg(dataOffset + SMPL_LOOP, std::ios::beg);
    header->loop = romStream.get();

    romStream.seekg(dataOffset + SMPL_SAMPLERATE, std::ios::beg);
    header->samplingRate = BYTEUTILS.getLittleEndian(romStream, 2);
    
    romStream.seekg(dataOffset + SMPL_LOOP_OFFSET, std::ios::beg);
    header->loopOffset = BYTEUTILS.getLittleEndian(romStream, 2);

    romStream.seekg(dataOffset + SMPL_SND_LENGTH, std::ios::beg);
    header->nonLoopLength = BYTEUTILS.getLittleEndian(romStream, 4);
    // Nur bis zum Anfang vom loop bereich und nicht bis ende von daten!!!!!!

    /*// Glaube nicht das das stimmt !
    if(header->type == 2) { // ima-adpcm uses blocks
        header->totalBlocks = (dataSize - HEADER_SIZE) / 36; // Frag mich nicht woher ich die 36 habe... (32 oder 36??)
    }*/ // EIN BLOCK... NUR EINEN FUCKING BLOCK HABEN SWAV'S IN SWAR ARCHIVEN!!!

    return true;
}


bool Swav::convert(std::vector<uint8_t>& outBuffer, uint16_t targetSampleRate,
                    int8_t semitonePitch) {
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    
    if(header.filesize > 0) { // Normal swav file // not used
        //sndType::Swav::Header& header = header;

    } else if(sampleHeader.nonLoopLength > 0) { // Swar swav sample
        SampleHeader* header = &sampleHeader;

        romStream.seekg(dataOffset + HEADER_SIZE, std::ios::beg);
        std::vector<uint8_t> buffer(dataSize - HEADER_SIZE);
        romStream.read((char*)buffer.data(), dataSize - HEADER_SIZE);

        // Data size * (2 samples per byte) * (stereo)
        std::vector<int16_t> pcmMonoData((dataSize - HEADER_SIZE) * 2);
        //std::vector<int16_t> pcmData(pcmMonoData.size() * 2);

        //                                               | Hat KEINEN Blockheader
        PCM.decodeImaAdpcm(buffer, pcmMonoData, 1, 0, 0, false);
        std::vector<int16_t> pcmData;

        {
            std::vector<int16_t> buffer;
            if(targetSampleRate <= 0) {
                buffer = std::move(pcmMonoData);
            } else {
                PCM.pitchInterpolatePcm16(pcmMonoData, buffer, header->samplingRate, targetSampleRate, semitonePitch);
            }

            // Sounds sind mono aber sdl ist stereo also müssen beide Seiten(l,r) die gleichen sounds haben
            pcmData.resize(buffer.size() * 2);
            for(size_t i = 0; i < buffer.size(); i++) {
                pcmData[2 * i] = buffer[i];
                pcmData[2 * i + 1] = buffer[i];
            }
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