#pragma once

#include <fstream>
#include <stdint.h>

#include "types.h"


class Sequencer {
private:
    sndType::Sseq sseq;
    sndType::Bank bnk;
    

public:
    struct Track {
        // Volume, Expression wirkt sich auch auf schon laufende noten aus!
        // Volume ist wie Lautstärkeregler am Verstärker und Expression wie Volumeregler am Midi-Keyboard

        // Relative to sseq start
        uint32_t offset = 0;
        uint32_t currOffset = 0;
        
        bool mono = true; // Mono-/Polyphone mode(mono = Wait for note to finish)
        uint8_t bmp = 0; // Max 240 BPM
        uint8_t vol = 0; // Max 127
        uint8_t pan = 0; // Max 127 // Stereo like panorama 0=left 64=middle 127=right
        uint8_t attack = 0; // Attack rate
        bool modulationDepth = false;
        uint8_t expression = 0; // Feinere volume angabe
        sndType::Swav swav;
        
    };

    // Array of tracks
    Track* tracks = nullptr;

    Sequencer(sndType::Sseq& sseq);

    ~Sequencer() {
        // Deletes whole array and not just first member
        delete[] this->tracks;
    }

    uint8_t trackCount = 0; // 0 = 1 Track

    bool programChange(uint8_t program, Track* track);

    // @param offset: Relative to sseq begin
    bool parseEvent(std::ifstream& in, uint32_t offset, Track* currTrack);
};