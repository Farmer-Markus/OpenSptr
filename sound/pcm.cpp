#include "pcm.h"
#include "../log.h"

#include <cstdint>
#include <vector>
#include <exception>
#include <algorithm>


// Used to decode IMA-ADPCM
int ima_index_table[16] = {
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8
}; 

// https://wiki.multimedia.cx/index.php/IMA_ADPCM
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

bool Pcm::decodeImaAdpcm(const std::vector<uint8_t>& blockData, std::vector<int16_t>& pcmData,
                            int channels, int side, size_t ignoredSamples, bool hasBlockHeader) {
    // 'side' Ob links oder rechts geschrieben werden muss. 'ignoredSamples' für looping
    // Block header:
    // Offset  |Size(bytes)
    // 0x0      2       Predictor
    // 0x2      1       Step index
    // 0x3      1       Unused
    // 0x4...   *       Compressed nibbles(1 nibble = 4 bits(0.5 bytes))

    int16_t predictor;
    int step_index;

    if(hasBlockHeader) {
        predictor = static_cast<int16_t>(blockData[0] | blockData[1] << 8); // Da es 2 Bytes sind und wegen little endian! byte shifting!
        step_index = static_cast<int>(blockData[2]);

        predictor = std::clamp(predictor, static_cast<int16_t>(-32768), static_cast<int16_t>(32767));
        step_index = std::clamp(step_index, 0, 88);

        if(ignoredSamples <= 0) {
            try { // Sicher ist sicher
                pcmData.at(side) = predictor; // Ersten direkt speichern
            } catch (const std::out_of_range& e) {
                LOG.err(e.what());
                return false;
            }

            side += channels;
        } else {
            ignoredSamples--;
            LOG.debug("Pcm::decodeImaAdpcm: Ignored samples: " + std::to_string(ignoredSamples));
        }
    } else {
        predictor = 0;
        step_index = 0;
    }
    //|
    //v
    //101010101 L
    //010101010 R
    // ^
    // |

    int bytes = blockData.size();
    int i;
    // Bei SWAV(aus SWAR) gibt es keine blockheader!!!!
    for(i = hasBlockHeader ? 4 : 0; i < bytes; i++) {
        for(uint8_t d = 0; d < 2; d++) { // 2 mal ausführen pro byte
            uint8_t nibble = (d == 0) ? (blockData[i] & 0x0F) : ((blockData[i] >> 4) & 0x0F); // Dann ist keine if schleife nötig
            int step = ima_step_table[step_index];

            // Anscheinend differenz berechnen :/ wtf thx https://problemkaputt.de/gbatek.htm#dsfilessoundsdatetc
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
                    LOG.info(std::to_string(i));
                    LOG.info("... of");
                    LOG.info(std::to_string(bytes));
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

bool Pcm::convertPcm8ToPcm16(const std::vector<uint8_t>& blockData, std::vector<int16_t>& pcmData,
                            int channels, int side, size_t ignoredSamples) {

    size_t bytes = blockData.size();
    for(size_t i = 0; i < bytes; i++) {
        if(ignoredSamples <= 0) {
            try {
                pcmData[side] = static_cast<int16_t>((blockData[i] - 128) << 8);
            } catch (const std::out_of_range& e) {
                LOG.err(e.what());
                return false;
            }
            side += channels;
        } else {
            LOG.debug("Pcm::convertPcm8ToPcm16: Ignored samples: " + std::to_string(ignoredSamples));
            ignoredSamples--;
        }
    }
    return true;
}

bool Pcm::interleavePcm16(const std::vector<int16_t>& blockData, std::vector<int16_t>& pcmData,
                            int channels, int side, size_t ignoredSamples) {

    size_t bytes = blockData.size();
    for(size_t i = 0; i < bytes; i++) {
        if(ignoredSamples <= 0) {
            try {
                pcmData[side] = blockData[i];
            } catch (const std::out_of_range& e) {
                LOG.err(e.what());
                return false;
            }
            side += channels;
        } else {
            LOG.debug("Pcm::convertPcm8ToPcm16: Ignored samples: " + std::to_string(ignoredSamples));
            ignoredSamples--;
        }
    }
    return true;
}

bool Pcm::interpolatePcm16(const std::vector<int16_t>& sndData, std::vector<int16_t>& outData,
        uint16_t sndSamplerate, uint32_t outSamplerate) {
    //

    float ratio = static_cast<float>(sndSamplerate) / static_cast<float>(outSamplerate);
    float cursor = 0; // = x

    while(static_cast<size_t>(cursor + 1) < sndData.size()) {
        size_t x1 = static_cast<size_t>(cursor);
        size_t x2 = x1 + 1;
        
        float y1 = sndData[x1];
        float y2 = sndData[x2];

        float sample = sndData[x1] + ((cursor - x1) / (x2 - x1) * (sndData[x2] - sndData[x1]));
        
        // Nur zur Sicherheit...
        sample = std::clamp(sample, -32768.0f, 32767.0f);
        outData.push_back(static_cast<int16_t>(sample));
        
        cursor += ratio;
    }

    return true;
}