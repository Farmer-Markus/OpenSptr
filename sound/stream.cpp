#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <mutex>

#include <SDL2/SDL.h>

#include "stream.h"
#include "types.h"
#include "sound.h"
#include "../byteutils.h"
#include "../filesystem.h"
#include "../log.h"
#include "../settings.h"


#define ID 0x0                     // 4 Bytes "STRM"
#define FILESIZE 0x8               // 4 Bytes
#define TYPE 0x18                  // 1 Byte
#define LOOP 0x19                  // 1 Byte
#define CHANNELS 0x1A              // 1 Byte
#define SAMPLING_RATE 0x1C         // 2 Bytes
#define LOOP_OFFSET 0x20           // 4 Bytes
#define TOTAL_SAMPLES 0x24         // 4 Bytes
#define WAVE_DATA_OFFSET 0x28      // 4 Bytes
#define TOTAL_BLOCKS 0x2C          // 4 Bytes
#define BLOCK_LENGTH 0x30          // 4 Bytes
#define SAMPLES_BLOCK 0x34         // 4 Bytes <Samples Per Block>
#define LAST_BLOCK_LENGTH 0x38     // 4 Bytes
#define SAMPLES_LAST_BLOCK 0x3C    // 4 Bytes <Samples Per Last Block>
#define DATA_SIZE 0x64             // 4 Bytes
#define DATA_OFFSET 0x68           // The wave data begin ...



bool Stream::getHeader(sdatType::STRM& strm) {
    sdatType::STRM::Header& header = strm.header;
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(strm.dataOffset, std::ios::beg);
        
    // Read header ang get Information
    header.id = BYTEUTILS.getBytes(romStream, 4);

    if(header.id != 0x5354524D) {
        LOG.hex("Failed to load STRM, wrong header ID:", header.id);
        return false;
    }

    romStream.seekg(strm.dataOffset + FILESIZE, std::ios::beg);
    header.filesize = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + TYPE, std::ios::beg);
    header.type = static_cast<uint8_t>(romStream.get());

    romStream.seekg(strm.dataOffset + LOOP, std::ios::beg);
    header.loop = static_cast<uint8_t>(romStream.get());

    romStream.seekg(strm.dataOffset + CHANNELS, std::ios::beg);
    header.channels = static_cast<uint8_t>(romStream.get());

    romStream.seekg(strm.dataOffset + SAMPLING_RATE, std::ios::beg);
    header.samplingRate = BYTEUTILS.getLittleEndian(romStream, 2);
        
    romStream.seekg(strm.dataOffset + LOOP_OFFSET, std::ios::beg);
    header.loopOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + TOTAL_SAMPLES, std::ios::beg);
    header.totalSamples = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + WAVE_DATA_OFFSET, std::ios::beg);
    header.waveDataOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + TOTAL_BLOCKS, std::ios::beg);
    header.totalBlocks = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + BLOCK_LENGTH, std::ios::beg);
    header.blockLength = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + SAMPLES_BLOCK, std::ios::beg);
    header.samplesBlock = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + LAST_BLOCK_LENGTH, std::ios::beg);
    header.lastBlockLength = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + SAMPLES_LAST_BLOCK, std::ios::beg);
    header.samplesLastBlock = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(strm.dataOffset + DATA_SIZE, std::ios::beg);
    header.dataSize = BYTEUTILS.getLittleEndian(romStream, 4);

    return true;
}

// LÖSCHEN
bool Stream::convert(sdatType::STRM strm, std::vector<uint8_t>& sound) {
    sound.clear();
    
    sdatType::STRM::Header& header = strm.header;
    getHeader(strm);
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();

    // Two vectors used for stereo sound. left/right
    std::vector<int16_t> wavData[2];
    romStream.seekg(strm.dataOffset + DATA_OFFSET, std::ios::beg);
    if(header.type == 0) { // PCM8 
        for(uint32_t i = 0; i < header.totalBlocks; i++) {
            for(uint8_t lr = 0; lr < header.channels; lr++) { // lr = left, right :D ()
                uint8_t data;
                romStream.read((char*)&data, 1);
                wavData[lr].push_back(static_cast<int16_t>(data - 128) << 8);
            } // Nach dem code wird erst lr++ ausgeführt... ups
        }

    } else if(header.type == 1) { // PCM16
        for(uint32_t i = 0; i < header.totalBlocks; i++) {
            for(uint8_t lr = 0; lr < header.channels; lr++) {
                uint16_t data = BYTEUTILS.getLittleEndian(romStream, 2); // Ja little endian ist richtig
                wavData[lr].push_back(data);
            }
        }

    } else { // IMA-ADPCM ... | hell nah... WHY NINTENDO WHY!?!
        wavData[0].reserve(header.totalBlocks * (header.blockLength * 4));
        wavData[1].reserve(header.totalBlocks * (header.blockLength * 4));
        for(uint32_t i = 0; i < header.totalBlocks; i++) {
            for(uint8_t lr = 0; lr < header.channels; lr++) {
                // Letzter Block kann kleiner sein!
                uint32_t blockLength = (i == header.totalBlocks - 1) ? header.lastBlockLength : header.blockLength;
                
                std::vector<uint8_t> block(blockLength);
                romStream.read((char*)block.data(), blockLength);
                decodeBlock(block, wavData[lr]);
                block.clear();
                LOG.info("Total Blocks: " + std::to_string(header.totalBlocks));
                LOG.info("Current Block: " + std::to_string(i));
            }
        }
    }

    size_t samples = wavData[0].size();
    std::vector<int16_t> finalBuffer;
    finalBuffer.reserve(samples * header.channels);

    if(header.channels > 1) {
        for(size_t i = 0; i < samples; i++) {
            finalBuffer.push_back(wavData[0][i]);
            finalBuffer.push_back(wavData[1][i]);
        }
    } else {
        finalBuffer = std::move(wavData[0]);
    }
    

    // 1. Header befüllen8
    WAVHeader wavHeader;
    wavHeader.chunkSize = 36 + header.dataSize;  // dataSize = Anzahl Samples * Kanäle * 2 (Bytes)
    wavHeader.numChannels = header.channels;
    wavHeader.sampleRate = header.samplingRate;
    wavHeader.byteRate = header.samplingRate * header.channels * (wavHeader.bitsPerSample / 8);
    wavHeader.blockAlign = header.channels * (wavHeader.bitsPerSample / 8);
    wavHeader.dataSize = static_cast<uint32_t>(finalBuffer.size()) * sizeof(int16_t);

    size_t size = sizeof(WAVHeader) + wavHeader.dataSize;

    sound.resize(size);
    // copy header
    std::memcpy(sound.data(), &wavHeader, sizeof(WAVHeader));
    // copy samples
    std::memcpy(sound.data() + sizeof(WAVHeader), finalBuffer.data(), wavHeader.dataSize);

    wavData[0].clear();
    wavData[0].shrink_to_fit();
    wavData[1].clear();
    wavData[1].shrink_to_fit();
    finalBuffer.clear();
    finalBuffer.shrink_to_fit();

    return true;
}

// Used to decode STRM
int ima_index_table[16] = {
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8
}; 

int ima_step_table[89] = {
  7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
  19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
  50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
  130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
  337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
  876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
  2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
  5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
}; 


// LÖSCHEN
void Stream::decodeBlock(const std::vector<uint8_t>& blockData, std::vector<int16_t>& pcmData) {
    // Block header:
    // Offset  |Size(bytes)
    // 0x0      2       Predictor
    // 0x2      1       Step index
    // 0x3      1       Unused
    // 0x4...   *       Compressed nibbles(1 nibble = 4 bits(0.5 bytes))

    int16_t predictor = blockData[0] | blockData[1] << 8; // Da es 2 Bytes sind und wegen little endian! byte shifting!
    pcmData.push_back(predictor); // Ersten direkt speichern
    int step_index = static_cast<int>(blockData[2]);

    int bytes = blockData.size();
    for(int i = 4; i < bytes; i++) {
        for(uint8_t d = 0; d < 2; d++) { // 2 mal ausführen pro byte
            uint8_t nibble = (d == 0) ? (blockData[i] & 0x0F) : ((blockData[i] >> 4) & 0x0F); // Dann ist keine if schleife nötig
            int step = ima_step_table[step_index];

            // Anscheinend differenz berechnen :/ wtf thx chatgpt
            int diff = step >> 3;
            if (nibble & 1) diff += step / 4;
            if (nibble & 2) diff += step / 2;
            if (nibble & 4) diff += step;
            if (nibble & 8) diff = -diff;

            predictor += diff;
            predictor = std::clamp(predictor, static_cast<int16_t>(-32768), static_cast<int16_t>(32767));

            step_index += ima_index_table[nibble & 0x0F];
            step_index = std::clamp(step_index, 0, 88);

            pcmData.push_back(predictor);
        }
    }
}


bool Stream::decodeBlocks(const std::vector<uint8_t>& blockData, std::vector<int16_t>& pcmData,
                            int channels, int side, size_t ignoredSamples) {
    // 'side' Ob links oder rechts geschrieben werden muss. 'ignoredSamples' für looping
    // Block header:
    // Offset  |Size(bytes)
    // 0x0      2       Predictor
    // 0x2      1       Step index
    // 0x3      1       Unused
    // 0x4...   *       Compressed nibbles(1 nibble = 4 bits(0.5 bytes))

    int16_t predictor = blockData[0] | blockData[1] << 8; // Da es 2 Bytes sind und wegen little endian! byte shifting!
    int step_index = static_cast<int>(blockData[2]);

    if(ignoredSamples <= 0) {
        try { // Sicher ist sicher
            pcmData.at(side) = predictor;
        } catch (const std::out_of_range& e) {
            LOG.err(e.what());
            return false;
        }
        
        pcmData[side] = predictor; // Ersten direkt speichern
        side += channels;
    } else {
        ignoredSamples--;
        LOG.debug("Stream::decodeBlocks: Ignored samples: " + std::to_string(ignoredSamples));
    }
    //|
    //v
    //101010101 L
    //010101010 R
    // ^
    // |

    int bytes = blockData.size();
    for(int i = 4; i < bytes; i++) {
        for(uint8_t d = 0; d < 2; d++) { // 2 mal ausführen pro byte
            uint8_t nibble = (d == 0) ? (blockData[i] & 0x0F) : ((blockData[i] >> 4) & 0x0F); // Dann ist keine if schleife nötig
            int step = ima_step_table[step_index];

            // Anscheinend differenz berechnen :/ wtf thx chatgpt
            int diff = step >> 3;
            if (nibble & 1) diff += step / 4;
            if (nibble & 2) diff += step / 2;
            if (nibble & 4) diff += step;
            if (nibble & 8) diff = -diff;

            predictor += diff;
            predictor = std::clamp(predictor, static_cast<int16_t>(-32768), static_cast<int16_t>(32767));

            step_index += ima_index_table[nibble & 0x0F];
            step_index = std::clamp(step_index, 0, 88);

            if(ignoredSamples <= 0) {
                try {
                    pcmData.at(side) = predictor;
                } catch (const std::out_of_range& e) {
                    LOG.err(e.what());
                    return false;
                }
                side += channels; // Damit bei mono nichts übersprungen wird
            } else {
                LOG.debug("Stream::decodeBlocks: Ignored samples: " + std::to_string(ignoredSamples));
                ignoredSamples--;
            }
        }
    }
    return true;
}


bool Stream::updateBuffer(Soundsystem::StrmSound& sound, int len) {
    size_t ignoredSamples = 0;
    if(sound.blockPosition >= sound.strm.header.totalBlocks) {
        if(sound.strm.header.loop <= 0)
            return false; // Just to make sure no crackling happens
        
        LOG.debug("Stream::updateBuffer: 'loop' is set to 'true'! Looping on loopoffset: " +
                    std::to_string(sound.strm.header.loopOffset));

        // loopOffset wird in Samples angegeben, deshalb block rausfinden und dann samples ignorieren
        sound.blockPosition = static_cast<int>(sound.strm.header.loopOffset /
                                                sound.strm.header.samplesBlock);
        ignoredSamples = sound.strm.header.loopOffset - (sound.blockPosition * sound.strm.header.samplesBlock);
    }


    sdatType::STRM& strm = sound.strm;
    sdatType::STRM::Header& header = sound.strm.header;
    std::vector<uint8_t>& outBuffer = sound.buffer;
    std::ifstream& romStream = FILESYSTEM.getRomStream();

    uint32_t blockLength = (sound.blockPosition == header.totalBlocks -1) ?
                                    header.lastBlockLength : header.blockLength;

    // blockLength - 4(block header) * 2(aus jedem byte von block werden 2 werte + 2(1 wert ist im 
    // header deffiniert(Keine ahnung warum man dann aber +2 machen muss und nicht +1...)))
    std::vector<int16_t> pcmData(header.channels * ((blockLength - 4) * 2) + 2 - ignoredSamples);
    // SEHR WICHTIG, dass buffer genau so groß wie daten sind!!
    
    if(SETTINGS.cacheSounds) { // When loading rawData to buffer -> Decoding on the fly
        if(strm.rawData.empty()){
            // Wenn buffer mit Rohdaten leer ist, dann einlesen
            strm.rawData.resize(strm.dataSize);
            romStream.seekg(strm.dataOffset + DATA_OFFSET, std::ios::beg);
            romStream.read((char*)strm.rawData.data(), static_cast<std::streamsize>(strm.dataSize));
        }

        if(header.type == 0) { // PCM8
            LOG.err("PCM8 Audio not supported yet!");
            return false;
        } else if(header.type == 1) { // PCM16
            LOG.err("PCM16 Audio not supported yet!");
            return false;
        } else if(header.type == 2) { // IMA-ADPCM ... | hell nah... WHY NINTENDO WHY!?!
            for(uint8_t lr = 0; lr < header.channels; lr++) {
                std::vector<uint8_t> block(blockLength);
                size_t offset = sound.blockPosition * header.channels * header.blockLength + lr * header.blockLength;
                std::memcpy(block.data(), strm.rawData.data() + offset, blockLength);
                decodeBlocks(block, pcmData, header.channels, lr, ignoredSamples);
            }
        } else {
            LOG.err("Stream::updateBuffer: header.type = " + std::to_string(header.type));
            return false;
        }

    } else { // If directly streaming from disc -> Decoding on the fly
        romStream.seekg(strm.dataOffset + DATA_OFFSET + (sound.blockPosition * blockLength * header.channels), std::ios::beg);
        if(header.type == 0) {
            LOG.err("PCM8 Audio not supported yet!");
            return false;

        } else if(header.type == 1) {
            LOG.err("PCM16 Audio not supported yet!");
            return false;

        } else if(header.type == 2) {
            for(uint8_t lr = 0; lr < 2; lr++) {
                std::vector<uint8_t> block(blockLength);
                romStream.read((char*)block.data(), blockLength);
                decodeBlocks(block, pcmData, header.channels, lr, ignoredSamples);
            }
        } else {
            LOG.err("Stream::updateBuffer: header.type = " + std::to_string(header.type));
            return false;
        }
    }

    sound.blockPosition++;

    size_t pcmDataSize = pcmData.size();
    size_t outBufferSize = outBuffer.size();

    outBuffer.resize(outBufferSize + pcmDataSize * sizeof(int16_t));
    std::memcpy(outBuffer.data() + outBufferSize, pcmData.data(), pcmDataSize * sizeof(int16_t));

    return true;
}