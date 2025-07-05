#pragma once

#include <fstream>
#include <stdint.h>
#include <variant>
#include <vector>
#include <unordered_map>

#include "sseq.h"
#include "bnk.h"
#include "swav.h"


class Sequencer {
private:
    Sseq sseq;

    struct HashHelper {
        std::size_t operator()(const std::pair<uint16_t, uint16_t>& in) const noexcept {
            // Unordered map braucht einen einzigen wert als index also packen wir 2 * 16 bit werte
            // zu einem 32 bit wert zusammen und benutzen das als index
            return static_cast<size_t>(in.first << 16 | in.second);
        }
    };

public:

    Bnk bnk;
    
    // Swavs werden unter dem selben index wie in der SWAR gecached!
    // Instrument Cache mit Key = Paar (swarIndex, swavIndex)
    std::unordered_map<std::pair<uint16_t, uint16_t>, Swav, HashHelper> instruments;


    struct Program {
        uint8_t frecord = 0;
        std::variant<Bnk::Record0, Bnk::RecordUnder16,
                    Bnk::Record16, Bnk::Record17> record;
    };

    struct Note {
        uint8_t absKey = 0;
        uint8_t velocity = 0;

        // Wie lange noch abgespielt werden soll
        // Wird einfach bei jedem tick -1 gerechnet
        size_t durationRemaining = 0;

        // Only for mixer
        size_t playPosition = 0;
        std::vector<uint8_t> sndData;
        uint16_t loopOffset = 0;
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

        Program program;
        std::vector<Note> activeNotes;
        
        Swav swav;
    };

    // Array of tracks
    // Track* tracks = nullptr;

    std::unordered_map<uint8_t, Track> tracks;
    std::ifstream romStream;

    // True on init success
    bool initSuccess = false;
    Sequencer(Sseq& sseq);

    ~Sequencer() {
        // Deletes whole array and not just first member
        //delete[] this->tracks;
        romStream.close();
    }

    uint8_t trackCount = 0;

    uint8_t bpm = 0; // Max 240 BPM
    uint16_t bpmTimer = 0;

    //bool finished = false; // NOCH ENTFERNEN!!

    bool tick();
    bool programChange(uint8_t program, Track* track);

    // @param offset: Relative to sseq begin
    bool parseEvent(uint32_t offset, Track* currTrack);
};