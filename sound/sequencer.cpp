#include <fstream>
#include <stdint.h>

#include "sequencer.h"

#include "sdat.h"
#include "sseq.h"
#include "bnk.h"
#include "swav.h"

#include "../log.h"
#include "../filesystem.h"
#include "../byteutils.h"


//bool Sequencer::

Sequencer::Sequencer(Sseq& sseq) {
    this->sseq = sseq;

    SDAT.getBnk(this->bnk, sseq.infoEntry.bnk);
    if(!bnk.getHeader())
        return;

    if(!bnk.parse())
        return;

    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(sseq.dataOffset + sseq.header.dataOffset);

    // Einlesen wie viele tracks es gibt oder ob der song direkt startet(1 track)
    if(romStream.get() == 0xFE) { // Multitrack
        uint16_t tracks = static_cast<uint16_t>(BYTEUTILS.getBytes(romStream, 2));
        // Last bit unused(I think...)
        for(uint8_t i = 0; i < 15; i++) {
            // Bit masking
            if(tracks & (1 << i)) {
                trackCount++;
            }
        }

    } else {} // Singletrack
    
    // Wenn keine trackpointer existieren dann existiert auch nur ein track
    // Aber wenn trackPointer da sind zeigen sie track0 nicht an!
    trackCount++;
    LOG.info("TrackCount: " + std::to_string(trackCount));
    /*if(!trackCount) {
        trackCount = 1;
    } else {

    }*/

    this->tracks = new Track[trackCount];

    if(trackCount == 1) {
        tracks[0].offset = sseq.header.dataOffset;
        tracks[0].currOffset = sseq.header.dataOffset;

    } else {
        Track* track = &tracks[0];
        uint32_t currOffset = sseq.header.dataOffset + 3 + (trackCount - 1)* 5;
        track->offset = currOffset;
        track->currOffset = currOffset;

        //                                  |Da track0 nicht als pointer gelistet ist!
        for(uint8_t i = 0; i < trackCount - 1; i++) {
            //                                           | General track infos     
            //                                           |    | Info how many tracks..
            //                                           |    |   | Size of track pointer
            currOffset = sseq.header.dataOffset + 3 + (i * 5);
            parseEvent(romStream, currOffset, nullptr);
        }
        LOG.info("done");
    }
}

bool Sequencer::tick() {
    /*if(finished) // NOCH ENTFERNEN!!
        return false;*/
    
    std::ifstream& stream = FILESYSTEM.getRomStream();
    
    for(uint8_t i = 0; i < trackCount; i++) {
        Track& track = tracks[i];
        if(track.finished)
            continue;

        if(track.restRemaining > 0) {
            //LOG.info("Resting for " + std::to_string(track.restRemaining));
            track.restRemaining--;
            continue;
        }


        if(!track.mode) { // Monophone mode
            if(!track.activeNotes.empty()) {
                if(track.activeNotes.size() > 1) {
                    LOG.err("Sequencer::tick: Mono track should NOT have multiple notes playing at once!");
                    return false;
                }

                if(track.activeNotes[0].durationRemaining > 0) {
                    track.activeNotes[0].durationRemaining--;
                    continue;
                } else {
                    track.activeNotes.erase(track.activeNotes.begin());
                    continue;
                }
            }

        } else { // Polyphone mode
            for(size_t n = 0; n < track.activeNotes.size(); n++) {
                Note& note = track.activeNotes[n];
                if(note.durationRemaining > 0) {
                    note.durationRemaining--;
                } else {
                    track.activeNotes.erase(track.activeNotes.begin() + n);

                    // Damit die nächste note, die nachrutscht auch abgefragt wird
                    n--;
                }
            }
        }

        // BPM berücksichtigen
        // https://www.feshrine.net/hacking/doc/nds-sdat.html#sseq at "2.1 Description"
        if(bpmTimer > 240 || bpm == 0) {
            if(bpm != 0)
                bpmTimer -= 240;
            // Nächstes event aus der sseq lesen & verarbeiten
            //LOG.info("Parsing Event...");
            if(!parseEvent(stream, track.currOffset, &tracks[i])) {
                track.finished = true;
                //finished = true;
                continue;
            }
        }
        //LOG.info("BpmTimer: " + std::to_string(bpmTimer));
        bpmTimer += bpm;
    }

    return true;
}

bool Sequencer::programChange(uint8_t program, Track* track) {
    // Am besten den ganzen Sequencer zum mixer geben.
    // Vielleicht nur die program number im track angeben und der mixer macht das, was 
    // grad hier passiert und lädt dann die samples, pitcht die usw.
    // An multithreading denken, verschiedene Romstreams oder threadsicher bla bla
    //
    uint8_t frecord = bnk.header.records[program].fRecord;
    LOG.info("Program change to frecord: " + std::to_string(frecord));
    
    if(frecord < 16 && frecord > 0) {
        Bnk::RecordUnder16& record = std::get<Bnk::RecordUnder16>(bnk.parsedInstruments[program]);
        // Am besten gazen record daten irgendwie in den track schreiben und alles andere im mixer machen

    } else if(frecord == 16) {
        Bnk::Record16* record = &std::get<Bnk::Record16>(bnk.parsedInstruments[program]);


    } else if(frecord == 17) {
        Bnk::Record17* record = &std::get<Bnk::Record17>(bnk.parsedInstruments[program]);


    } else {
        LOG.debug("Sequencer::programChange: Found Wrong frecord in bnk!");
        return false;
    }

    return true;
}

bool Sequencer::parseEvent(std::ifstream& in, uint32_t offset, Track* currTrack) {
    in.seekg(sseq.dataOffset + offset, std::ios::beg);

    uint8_t byte = 0;
    byte = static_cast<uint8_t>(in.get());
    /*LOG.hex("Current:", byte);
    LOG.hex("Position:", in.tellg());*/
    

    if(byte < 0x80) { // Note one Event
        LOG.info("NOTE EVENT");
        Note note;
        note.absKey = byte;
        note.velocity = in.get(); // 0 - 127 // https://audiodramaproduction.com/audio-terms-glossary/sound-velocity/

        //uint8_t duration = in.get();
        while(true) { // Same as rest event (can be multiple bytes long)
            byte = in.get();
            note.durationRemaining += byte;

            if(!(byte & 0x80)) { // Wenn erstes bit nicht true(1) ist folgt kein weiterer wert!
                break;
            }
        }

        // Note adden
        currTrack->activeNotes.push_back(note);

    } else {
        switch(byte) {
            // Moved to Sequencer initialisation
            /*case 0xFE: {
                LOG.info("TRACKS USED EVENT");
                
                // Which tracks are used ...ist es nen multiTrack. die 2 bytes danach sind die anzahl der tracks(immer 1 zu viel...)
                //in.seekg(2, std::ios::cur);
                uint16_t tracks = static_cast<uint16_t>(BYTEUTILS.getBytes(in, 2));
                // Last bit unused(I think...)
                for(uint8_t i = 0; i < 15; i++) {
                    // Bit masking
                    if(tracks & (1 << i)) {
                        trackCount++;
                    }
                }
                
                break;
            }*/
            
            // Wenn erstes bit vom byte = 1 ist dann kommt noch ein byte(und wenn das erste byte ... immer weiter)
            case 0x80: {
                LOG.info("WAIT EVENT");
                while(true) {
                    byte = in.get();
                    currTrack->restRemaining += byte;
                    if(!(byte & 0x80)) { // Wenn erstes bit nicht true(1) ist folgt kein weiterer wert!
                        break;
                    }
                }

                break;
            }

            case 0x81:
                LOG.info("PROGRAM CHANGE EVENT");
                // Program change to in.get()
                //LOG.info("0x81");
                //in.seekg(1, std::ios::cur);
                if(!programChange(in.get(), currTrack))
                    return false;

                break;
            
            // Open track whatever... in.get(4)
            // Track pointer (first byte track nr)
            case 0x93: {
                LOG.info("TRACK POINTER EVENT");
                //in.seekg(4, std::ios::cur);
                Track* track = &tracks[in.get()];
                uint32_t trackOffset = BYTEUTILS.getLittleEndian(in, 3) + sseq.header.dataOffset;
                track->offset = trackOffset;
                track->currOffset = trackOffset;

                break;
            }
            
            case 0x94: // ing.get(3)
                in.seekg(3, std::ios::cur);
                break;
            
            case 0x95: // in.get(3)
                LOG.info("CALL EVENT!!!!!!!!!!!!!!!!!!!!!!!!!!");

                //                                                       | es werden ja jetzt noch 3 bytes eingelesen 
                // und der eine byte am anfang wurde auch noch nicht dazu|gerechnet!
                currTrack->callAddress.push_back(currTrack->currOffset + 4);
                currTrack->currOffset = BYTEUTILS.getLittleEndian(in, 3) + sseq.header.dataOffset;
                //in.seekg(3, std::ios::cur);
                return true;
                break;
            
            case 0xFD: // in.get(3)
                LOG.info("RETURN EVENT----------------------------");

                if(currTrack->callAddress.empty()) {
                    LOG.err("Sequencer::parseEvent: Reached return statement in SSEQ but no callAddress is set!");
                    return false;
                }

                currTrack->currOffset = currTrack->callAddress.back();
                currTrack->callAddress.pop_back(); // Letztes element löschen
                return true;
                break;
            
            case 0xC0:
                LOG.info("PAN EVENT");
                //in.seekg(1, std::ios::cur);
                currTrack->pan = in.get(); // Max 127
                // add Pan in.get()
                break;
            
            case 0xC1:
                LOG.info("VOLUME EVENT");
                //in.seekg(1, std::ios::cur);
                currTrack->vol = in.get(); // Max 127
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
                LOG.info("PITCH BEND EVENT");
                //in.seekg(1, std::ios::cur);
                currTrack->pitchBend = in.get();
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
                LOG.info("TRACK MODE EVENT");
                //in.seekg(1, std::ios::cur);

                currTrack->mode = in.get();
                // Mono/Poly mode Monophone(0)=(Eine note gleichzeitig)/Polyphone(1)=(mehrere noten gleichzeitg erlaubt)
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
                LOG.info("MODULATION DEPTH EVENT");
                //in.seekg(1, std::ios::cur);
                currTrack->modulationDepth = in.get();
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
                LOG.info("ATTACK RATE EVENT");
                //in.seekg(1, std::ios::cur);
                currTrack->attack = in.get();
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
                LOG.info("EXPRESSION EVENT");
                //in.seekg(1, std::ios::cur);
                currTrack->expression = in.get();
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
                LOG.info("BPM EVENT");
                //in.seekg(2, std::ios::cur);
                bpm = BYTEUTILS.getLittleEndian(in, 2);
                // TEMPO(BMP) in.get(2)
                break;
            
            case 0xE3:
                in.seekg(2, std::ios::cur);
                // SWEEP PITCH in.get(2)
                break;

            case 0xFF:
                // End of Track!
                LOG.info("SSEQ Parser: End of Track, returning");
                return false;
                break;
            
            default:
                LOG.info("SSEQ Parser: Found weird value");
                LOG.hex("Value:", byte);
                return false;
                break;

        }
    }
    
    if(currTrack != nullptr)
        currTrack->currOffset += static_cast<uint32_t>(in.tellg()) - (sseq.dataOffset + offset);

    return true;
}