#pragma once

#include <fstream>
#include <stdint.h>
#include <variant>
#include <vector>

#include "sseq.h"
#include "bnk.h"
#include "swav.h"


class Sequencer {
private:
    Sseq sseq;    

public:
    struct Note {
        uint8_t absKey = 0;
        uint8_t velocity = 0;

        // Wie lange noch abgespielt werden soll
        // Wird einfach bei jedem tick -1 gerechnet
        size_t durationRemaining = 0;
    };

    struct Track {
        bool finished = false;
        // Volume, Expression wirkt sich auch auf schon laufende noten aus!
        // Volume ist wie Lautstärkeregler am Verstärker und Expression wie Volumeregler am Midi-Keyboard

        // Relative to sseq start
        uint32_t offset = 0; // Wird vielleicht garnicht gebraucht
        uint32_t currOffset = 0;
        
        // true = polyphone; false = monophone;
        bool mode = false; // Mono-/Polyphone mode(mono = Wait for note to finish)
        uint8_t vol = 0; // Max 127
        uint8_t pan = 0; // Max 127 // Stereo like panorama 0=left 64=middle 127=right
        uint8_t attack = 0; // Attack rate
        bool modulationDepth = false;
        uint8_t expression = 0; // Feinere volume angabe
        uint8_t pitchBend = 0;

        size_t restRemaining = 0;
        std::vector<uint32_t> callAddress;

        std::vector<Note> activeNotes;

        Swav swav;
    };

    // Array of tracks
    Track* tracks = nullptr;

    Sequencer(Sseq& sseq);

    ~Sequencer() {
        // Deletes whole array and not just first member
        delete[] this->tracks;
    }

    Bnk bnk;
    uint8_t trackCount = 0; // 0 = 1 Track

    uint8_t bpm = 0; // Max 240 BPM
    uint16_t bpmTimer = 0;

    //bool finished = false; // NOCH ENTFERNEN!!

    bool tick();
    bool programChange(uint8_t program, Track* track);

    // @param offset: Relative to sseq begin
    bool parseEvent(std::ifstream& in, uint32_t offset, Track* currTrack);
};