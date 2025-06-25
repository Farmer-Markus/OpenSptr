#include <fstream>
#include <stdint.h>

#include "sequencer.h"
#include "../log.h"


bool Sequencer::parseEvent(std::ifstream& in, uint32_t offset) {

    uint8_t byte = 0;
    byte = static_cast<uint8_t>(in.get());
    LOG.hex("Current:", byte);
    LOG.hex("Position:", in.tellg());

    if(byte < 0x80) { // Note one Event
        LOG.info("NOTE EVENT");
        uint8_t absKey = byte;
        uint8_t velocity = in.get(); // 0 - 127
        uint8_t duration = in.get();
        // Note adden
    } else {
        switch(byte) {
            case 0xFE:
                // Which traks are used ...ist es nen multiTrack. die 2 bytes danach sind die anzahl der tracks(immer 1 zu viel...)
                in.seekg(2, std::ios::cur);
                break;
            
            // Wenn erstes bit vom byte = 1 ist dann kommt noch ein byte(und wenn das erste byte ... immer weiter)
            case 0x80: {
                bool resting = false;

                while(!resting) {
                    byte = in.get();
                    if(byte & 0x80) { // Wenn erstes bit true(1) ist
                        LOG.info("LONGER RESTING VALUE!!!");
                    } else {
                        resting = true;
                    }
                }

                break;
            }

            case 0x81:
                // Program change to in.get()
                LOG.info("0x81");
                in.seekg(1, std::ios::cur);
                break;
            
            // Open track whatever... in.get(4)
            case 0x93:
                in.seekg(4, std::ios::cur);
                break;
            
            case 0x94: // ing.get(3)
                in.seekg(3, std::ios::cur);
                break;
            
            case 0x95: // in.get(3)
                in.seekg(3, std::ios::cur);
                break;
            
            case 0xC0:
                in.seekg(1, std::ios::cur);
                // add Pan in.get()
                break;
            
            case 0xC1:
                in.seekg(1, std::ios::cur);
                // Vol in.get()
                break;
            
            case 0xC2: 
                in.seekg(1, std::ios::cur);
                // Master volume in.get()
                break;
            
            case 0xC3: 
                in.seekg(1, std::ios::cur);
                // Transpose in.get()
                break;
            
            case 0xC4:
                in.seekg(1, std::ios::cur);
                // Pitch bend in.get()
                break;
            
            case 0xC5:
                in.seekg(1, std::ios::cur);
                // Pitch bend range in.get()
                break;
            
            case 0xC6:
                in.seekg(1, std::ios::cur);
                // Track Priority in.get()
                break;
            
            case 0xC7:
                in.seekg(1, std::ios::cur);
                // Mono/Poly mode Monophone(1)=(Eine note gleichzeitig)/Polyphone(0)=(mehrere noten gleichzeitg erlaubt)
                break;
            
            // Unknown [0: Off, 1: On] TIE
            case 0xC8:
                in.seekg(1, std::ios::cur);
                break;
            
            // Unknown PORTAMENTO CONTROL
            case 0xC9:
                in.seekg(1, std::ios::cur);
                break;
            
            case 0xCA:
                in.seekg(1, std::ios::cur);
                // MODULATION DEPTH  [0: Off, 1: On] in.get()
                break;
            
            case 0xCB:
                in.seekg(1, std::ios::cur);
                // MODULATION SPEED in.get()
                break;

            case 0xCC:
                in.seekg(1, std::ios::cur);
                // MODULATION TYPE [0: Pitch, 1: Volume, 2: Pan] in.get()
                break;
            
            case 0xCD:
                in.seekg(1, std::ios::cur);
                // MODULATION RANGE in.get()
                break;
            
            case 0xCE:
                in.seekg(1, std::ios::cur);
                // PORTAMENTO ON/OFF in.get()
                break;
            
            case 0xCF:
                in.seekg(1, std::ios::cur);
                // PORTAMENTO TIME in.get()
                break;
            
            case 0xD0:
                in.seekg(1, std::ios::cur);
                // ATTACK RATE in.get();
                break;
            
            case 0xD1:
                in.seekg(1, std::ios::cur);
                // DECAY RATE in.get()
                break;
            
            case 0xD2:
                in.seekg(1, std::ios::cur);
                // SUSTAIN RATE in.get()
                break;

            case 0xD3:
                in.seekg(1, std::ios::cur);
                // RELEASE RATE in.get()
                break;
            
            case 0xD4:
                in.seekg(1, std::ios::cur);
                // LOOP START MARKER (and how many times to be looped ( in.get() ) )
                break;
            
            case 0xFC:
                // LOOP END MARKER
                break;
            
            case 0xD5:
                in.seekg(1, std::ios::cur);
                // EXPRESSION in.get()
                break;

            case 0xD6:
                in.seekg(1, std::ios::cur);
                // PRINT VARIABLE (unknown) in.get()
                break;
            
            case 0xE0:
                in.seekg(2, std::ios::cur);
                // MODULATION DELAY in.get(2)
                break;
            
            case 0xE1:
                in.seekg(2, std::ios::cur);
                // TEMPO(BMP) in.get(2)
                break;
            
            case 0xE3:
                in.seekg(2, std::ios::cur);
                // SWEEP PITCH in.get(2)
                break;

            case 0xFF:
                // End of Track!
                LOG.info("SSEQ Parser: End of Track, returning");
                return true;
                break;
            
            default:
                LOG.info("SSEQ Parser: Found weird value");
                LOG.hex("Value:", byte);
                return false;
                break;

        }
    }

    return true;
}