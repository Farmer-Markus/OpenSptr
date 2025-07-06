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

    romStream.open(FILESYSTEM.getRomPath(), std::ios::binary);
    romStream.seekg(sseq.dataOffset + sseq.header.dataOffset);

    // Einlesen wie viele tracks es gibt oder ob der song direkt startet(1 track)
    if(romStream.get() == 0xFE) { // Multitrack
        uint16_t tracks = static_cast<uint16_t>(BYTEUTILS.getLittleEndian(romStream, 2));
        // Last bit unused(I think...)
        for(uint8_t i = 0; i < 16; i++) {
            // Bit masking
            if(tracks & (1 << i)) {
                trackCount++;
            }
        }

    } else {
        trackCount = 1;
    } // Singletrack
    
    // Wenn keine trackpointer existieren dann existiert auch nur ein track
    // Aber wenn trackPointer da sind zeigen sie track0 nicht an!
    //trackCount++;
    LOG.info("TrackCount: " + std::to_string(trackCount));
    /*if(!trackCount) {
        trackCount = 1;
    } else {

    }*/

    //this->tracks = new Track[trackCount];

    if(trackCount == 1) {
        tracks[0].offset = sseq.header.dataOffset;
        tracks[0].currOffset = sseq.header.dataOffset;

    } else {
        Track& track = tracks[0];
        uint32_t currOffset = sseq.header.dataOffset + 3 + (trackCount - 1)* 5;
        track.offset = currOffset;
        track.currOffset = currOffset;

        //                                  |Da track0 nicht als pointer gelistet ist!
        for(uint8_t i = 0; i < trackCount - 1; i++) {
            //break; //----------------------------------------------------------------------------------
            //                                    | General track infos     
            //                                    |    | Info how many tracks..
            //                                    |    |   | Size of track pointer
            currOffset = sseq.header.dataOffset + 3 + (i * 5);
            LOG.info("Processing Event for track: " + std::to_string(i));
            parseEvent(currOffset, nullptr);
        }
        //LOG.info("done");
    }

    // Lambda function
    auto cacheInstr = [](Sequencer* seq, uint16_t swarIndex, uint16_t swavIndex) {
        Swav swav;
        Swar swar;
        SDAT.getSwar(swar, swarIndex);
        swar.getHeader();
        swar.getSound(swav, swavIndex);
        swav.getSampleHeader();
        swav.read();

        LOG.debug("Sequencer: Caching instrument: " + std::to_string(swavIndex) + " Of SWAR: " + std::to_string(swarIndex));
        seq->instruments[{swarIndex, swavIndex}] = std::move(swav);
        LOG.debug("Done");
    };

    
    LOG.debug("Sequencer: Caching instruments..");
    // Alle samples aus der sseq vorladen
    for(uint32_t instrument = 0; instrument < bnk.header.totalInstruments; instrument++) {
        uint8_t frecord = bnk.header.records[instrument].fRecord;
        LOG.hex("Frecord:", frecord);

        if(frecord == 0) {
            // Unused
        } else if(frecord < 16 && frecord > 0) {
            Bnk::RecordUnder16& record = std::get<Bnk::RecordUnder16>(bnk.parsedInstruments[instrument]);
            // Am besten gazen record daten irgendwie in den track schreiben und alles andere im mixer machen
            cacheInstr(this, bnk.infoEntry.swar[record.swar], record.swav);

        } else if(frecord == 16) {
            Bnk::Record16& record = std::get<Bnk::Record16>(bnk.parsedInstruments[instrument]);
            for(Bnk::NoteDefine define : record.defines){
                cacheInstr(this, bnk.infoEntry.swar[define.swar], define.swav);
            }

        } else if(frecord == 17) {
            Bnk::Record17& record = std::get<Bnk::Record17>(bnk.parsedInstruments[instrument]);
            for(Bnk::NoteDefine define : record.defines){
                cacheInstr(this, bnk.infoEntry.swar[define.swar], define.swav);
            }

        } else {
            LOG.debug("Sequencer: Found Wrong frecord in bnk while caching insruments!");
            return;
        }

    }

    LOG.debug("Sequencer: Done caching");

    initSuccess = true;
}


// Am Anfang die BNK lesen und JEDES SWAV Sample in einen buffer laden.
// Dann für jede note, die abgespielt wird die sample daten mitgeben aber auch die note-id
//  Vielleicht auch bnk record als reference speichern

bool Sequencer::programChange(uint8_t program, Track* track) {
    // Am besten den ganzen Sequencer zum mixer geben.
    // Vielleicht nur die program number im track angeben und der mixer macht das, was 
    // grad hier passiert und lädt dann die samples, pitcht die usw.
    // An multithreading denken, verschiedene Romstreams oder threadsicher bla bla
    //
    uint8_t frecord = bnk.header.records[program].fRecord;
    track->program.frecord = frecord;
    LOG.info("Program change to frecord: " + std::to_string(frecord));
    
    if(frecord == 0) {
        // Unused
    } else if(frecord < 16 && frecord > 0) {
        track->program.record = std::get<Bnk::RecordUnder16>(bnk.parsedInstruments[program]);
        // Am besten gazen record daten irgendwie in den track schreiben und alles andere im mixer machen

    } else if(frecord == 16) {
        track->program.record = std::get<Bnk::Record16>(bnk.parsedInstruments[program]);

    } else if(frecord == 17) {
        track->program.record = std::get<Bnk::Record17>(bnk.parsedInstruments[program]);


    } else {
        LOG.debug("Sequencer: Found Wrong frecord in bnk while caching insruments!");
        return false;
    }

    return true;
}


bool Sequencer::tick() {
    if(!initSuccess) {
        LOG.err("Sequencer::tick: Sequencer was not initialized successfully. Init failed!.");
        return false;
    }

    /*if(finished) // NOCH ENTFERNEN!!
        return false;*/

    // BPM berücksichtigen
    // https://www.feshrine.net/hacking/doc/nds-sdat.html#sseq at "2.1 Description"
    bpmTimer += bpm;
    if(bpmTimer > 240) {
        if(bpm != 0)
            bpmTimer -= 240;
    } else {
        return true;
    }
    //LOG.info("BpmTimer: " + std::to_string(bpm));
    
    
    
    for(auto& [nr, track] : tracks) {
        //Track& track = tracks[i];
        if(track.finished)
            continue;

        /*if(!tracks[0].activeNotes.empty())
            LOG.info("Note: " + std::to_string(tracks[0].activeNotes[0].absKey));*/

        if(!track.mode) { // Monophone mode
            if(!track.activeNotes.empty()) {
                if(track.activeNotes.size() > 1) {
                    LOG.err("Sequencer::tick: Mono track should NOT have multiple notes playing at once!");
                    return false;
                }

                if(track.activeNotes[0].durationRemaining <= 0) {
                    track.activeNotes.clear();
                    //continue;
                } else {
                    track.activeNotes[0].durationRemaining--;
                    //continue;
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

        if(track.restRemaining > 0) {
            //LOG.info("Resting for " + std::to_string(track.restRemaining));
            track.restRemaining--;
            continue;
        }

        if(!parseEvent(track.currOffset, &track)) {
            track.finished = true;
            //finished = true;
            
            //continue;
        }
    }

    return true;
}

bool Sequencer::parseEvent(uint32_t offset, Track* currTrack) {
    romStream.seekg(sseq.dataOffset + offset, std::ios::beg);

    uint8_t byte = 0;
    byte = static_cast<uint8_t>(romStream.get());
    /*LOG.hex("Current:", byte);
    LOG.hex("Position:", romStream.tellg());*/

    if(byte < 0x80) { // Note one Event
        LOG.info("NOTE EVENT");
        Note note;
        note.absKey = byte;
        note.velocity = romStream.get(); // 0 - 127 // https://audiodramaproduction.com/audio-terms-glossary/sound-velocity/

        //uint8_t duration = romStream.get();
        do { // Same as rest event (can be multiple bytes long)
            byte = romStream.get();
            note.durationRemaining = (note.durationRemaining <<7) | (byte & 0x7F);
        } while(byte & 0x80);
        //note.durationRemaining *= 3;
        

        // Note adden
        if(!currTrack->mode)
            currTrack->activeNotes.clear();
        currTrack->activeNotes.push_back(note);
        /*if(currTrack->activeNotes.empty())
            currTrack->activeNotes.push_back(note);*/

    } else {
        switch(byte) {
            // Moved to Sequencer initialisation
            /*case 0xFE: {
                LOG.info("TRACKS USED EVENT");
                
                // Which tracks are used ...ist es nen multiTrack. die 2 bytes danach sind die anzahl der tracks(immer 1 zu viel...)
                //romStream.seekg(2, std::ios::cur);
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
                do {
                    byte = romStream.get();
                    currTrack->restRemaining = (currTrack->restRemaining <<7) | (byte & 0x7F);
                } while(byte & 0x80);
                  // Solange erstes bit = true ist
                LOG.err(std::to_string(currTrack->restRemaining));

                break;
            }

            case 0x81:
                LOG.info("PROGRAM CHANGE EVENT");
                // Program change to romStream.get()
                //LOG.info("0x81");
                //romStream.seekg(1, std::ios::cur);
                if(!programChange(romStream.get(), currTrack))
                    return false;

                break;
            
            // Open track whatever... romStream.get(4)
            // Track pointer (first byte track nr)
            case 0x93: {
                LOG.info("TRACK POINTER EVENT");
                //romStream.seekg(4, std::ios::cur);
                Track& track = tracks[romStream.get()];
                uint32_t trackOffset = BYTEUTILS.getLittleEndian(romStream, 3) + sseq.header.dataOffset;
                track.offset = trackOffset;
                track.currOffset = trackOffset;

                break;
            }
            
            // Jump/Loop
            case 0x94: // ing.get(3)
            LOG.info("JUMP EVENT");
                currTrack->currOffset = BYTEUTILS.getLittleEndian(romStream, 3) + sseq.header.dataOffset;
                return true;
                //romStream.seekg(3, std::ios::cur);
                break;
            
            case 0x95: // romStream.get(3)
                LOG.info("CALL EVENT");

                //                                                       | es werden ja jetzt noch 3 bytes eingelesen 
                // und der eine byte am anfang wurde auch noch nicht dazu|gerechnet!
                currTrack->callAddress.push_back(currTrack->currOffset + 4);
                currTrack->currOffset = BYTEUTILS.getLittleEndian(romStream, 3) + sseq.header.dataOffset;
                //romStream.seekg(3, std::ios::cur);
                return true;
                break;
            
            case 0xFD: // romStream.get(3)
                LOG.info("RETURN EVENT");

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
                //romStream.seekg(1, std::ios::cur);
                currTrack->pan = romStream.get(); // Max 127
                // add Pan romStream.get()
                break;
            
            case 0xC1:
                LOG.info("VOLUME EVENT");
                //romStream.seekg(1, std::ios::cur);
                currTrack->vol = romStream.get(); // Max 127
                // Vol romStream.get()
                break;
            
            case 0xC2: 
                LOG.info("MASTER-VOLUME EVENT");
                romStream.seekg(1, std::ios::cur);
                // Master volume romStream.get()
                break;
            
            case 0xC3: 
                romStream.seekg(1, std::ios::cur);
                // Transpose romStream.get()
                break;
            
            case 0xC4:
                LOG.info("PITCH BEND EVENT");
                //romStream.seekg(1, std::ios::cur);
                currTrack->pitchBend = romStream.get();
                // Pitch bend romStream.get()
                break;
            
            case 0xC5:
                romStream.seekg(1, std::ios::cur);
                // Pitch bend range romStream.get()
                break;
            
            case 0xC6:
                LOG.info("TRACK PRIORITY EVENT");
                currTrack->priority = romStream.get();
                //romStream.seekg(1, std::ios::cur);
                // Track Priority romStream.get()
                break;
            
            case 0xC7:
                LOG.info("TRACK MODE EVENT");
                //romStream.seekg(1, std::ios::cur);

                currTrack->mode = romStream.get();
                //currTrack->mode = (romStream.get() == 0);
                // Mono/Poly mode Monophone(0)=(Eine note gleichzeitig)/Polyphone(1)=(mehrere noten gleichzeitg erlaubt)
                break;
            
            // Unknown [0: Off, 1: On] TIE
            case 0xC8:
                romStream.seekg(1, std::ios::cur);
                break;
            
            // Unknown PORTAMENTO CONTROL
            case 0xC9:
                romStream.seekg(1, std::ios::cur);
                break;
            
            case 0xCA:
                LOG.info("MODULATION DEPTH EVENT");
                //romStream.seekg(1, std::ios::cur);
                currTrack->modulationDepth = romStream.get();
                // MODULATION DEPTH  [0: Off, 1: On] romStream.get()
                break;
            
            case 0xCB:
                romStream.seekg(1, std::ios::cur);
                // MODULATION SPEED romStream.get()
                break;

            case 0xCC:
                romStream.seekg(1, std::ios::cur);
                // MODULATION TYPE [0: Pitch, 1: Volume, 2: Pan] romStream.get()
                break;
            
            case 0xCD:
                romStream.seekg(1, std::ios::cur);
                // MODULATION RANGE romStream.get()
                break;
            
            case 0xCE:
                romStream.seekg(1, std::ios::cur);
                // PORTAMENTO ON/OFF romStream.get()
                break;
            
            case 0xCF:
                romStream.seekg(1, std::ios::cur);
                // PORTAMENTO TIME romStream.get()
                break;
            
            case 0xD0:
                LOG.info("ATTACK RATE EVENT");
                //romStream.seekg(1, std::ios::cur);
                currTrack->attack = romStream.get();
                // ATTACK RATE romStream.get();
                break;
            
            case 0xD1:
                romStream.seekg(1, std::ios::cur);
                // DECAY RATE romStream.get()
                break;
            
            case 0xD2:
                romStream.seekg(1, std::ios::cur);
                // SUSTAIN RATE romStream.get()
                break;

            case 0xD3:
                romStream.seekg(1, std::ios::cur);
                // RELEASE RATE romStream.get()
                break;
            
            case 0xD4:
                romStream.seekg(1, std::ios::cur);
                // LOOP START MARKER (and how many times to be looped ( romStream.get() ) )
                break;
            
            case 0xFC:
                // LOOP END MARKER
                break;
            
            case 0xD5:
                LOG.info("EXPRESSION EVENT");
                //romStream.seekg(1, std::ios::cur);
                currTrack->expression = romStream.get();
                // EXPRESSION romStream.get()
                break;

            case 0xD6:
                romStream.seekg(1, std::ios::cur);
                // PRINT VARIABLE (unknown) romStream.get()
                break;
            
            case 0xE0:
                romStream.seekg(2, std::ios::cur);
                // MODULATION DELAY romStream.get(2)
                break;
            
            case 0xE1:
                LOG.info("BPM EVENT------------------------------------------------");
                //romStream.seekg(2, std::ios::cur);
                bpm = BYTEUTILS.getLittleEndian(romStream, 2);
                // TEMPO(BMP) romStream.get(2)
                break;
            
            case 0xE3:
                romStream.seekg(2, std::ios::cur);
                // SWEEP PITCH romStream.get(2)
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
        currTrack->currOffset += static_cast<uint32_t>(romStream.tellg()) - (sseq.dataOffset + offset);

    return true;
}