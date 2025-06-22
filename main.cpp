#include <iostream>
#include <filesystem>

#include <fstream>
#include <ostream>
#include <cstring>
#include <vector>

#include <SDL2/SDL.h>

#include "log.h"

#include "filesystem.h"
#include "sound/sound.h"
#include "sound/types.h"
#include "byteutils.h"
#include "sound/audioId.h"
#include "sound/stream.h"
#include "sound/bank.h"
#include "sound/swar.h"
#include "sound/swav.h"
#include "shell/shell.h"
#include "sound/sseq.h"

//using namespace sndType;

void showHelp() {

}

int main(int argc, char* argv[]) {
    std::string path;
    std::string extractPath;
    bool shell = false;

    std::string* arg = nullptr;
    for(int i = 0; i < argc; i++) {
        if(i == 0)
            continue;
        
        if(arg != nullptr) {
            *arg = argv[i];
            arg = nullptr;
            continue;
        }


        if(!std::strcmp(argv[i], "--romPath")) {
            arg = &path;
            continue;
        }

        if(!std::strcmp(argv[i], "-s") || !std::strcmp(argv[i], "--shell")) {
            shell = true;
            if(!extractPath.empty()) {
                LOG.err("Cannot use '" + std::string(argv[i]) + "' and -x/--extract together!");
                return 1;
            }
            continue;
        }

        if(!std::strcmp(argv[i], "-x") || !std::strcmp(argv[i], "--extract")) {
            arg = &extractPath;
            if(shell) {
                LOG.err("Cannot use '" + std::string(argv[i]) + "-s/--shell together!");
                return 1;
            }
            continue;
        }

        if(!std::strcmp(argv[i], "-h") || !std::strcmp(argv[i], "-?") || !std::strcmp(argv[i], "--help")) {
            showHelp();
            return 0;
        }

        LOG.info("Argument : '" + std::string(argv[i]) + "' not found.");
        continue;
    }

    std::filesystem::path romPath(path);
    if(romPath.empty() ? FILESYSTEM.searchRom(romPath, ".") : FILESYSTEM.verifyRom(romPath)) {
        if(!FILESYSTEM.init(romPath)) {
            LOG.err("Failed to initialize filesystem");
            return 1;
        }
    } else {
        LOG.err("Failed to load rom.");
    }

    if(shell) {
        Shell shell;
        shell.enter();
        return 0;
    }

    if(!extractPath.empty()) {
        FILESYSTEM.extractRom("", extractPath);
    }
    
    // When not entering shell & ..
    if(!shell && SOUNDSYSTEM.loadSDAT("SoundData/final_sound_data.sdat"))
        SOUNDSYSTEM.init();

    sndType::Strm strm;
    SDAT.getStrm(strm, 7);
    STREAM.getHeader(strm);
    Soundsystem::StrmSound snd;
    snd.strm = strm;
    SOUNDSYSTEM.strmQueue.push_back(snd);
    LOG.info("STRM Samplerate: " + std::to_string(strm.header.samplingRate));

    sndType::Swar swar;
    SDAT.getSwar(swar, 0);
    SWAR.getHeader(swar);
    sndType::Swav wav;
    SWAR.getSound(swar, wav, 532); //300 //302 //311 //312 //318 //322 //376 //385 //386 //
    SWAV.getSampleHeader(wav);

    std::vector<uint8_t> buffer;
    SWAV.convert(wav, buffer, 32728);

    LOG.hex("Sound samplerate:", wav.sampleHeader.samplingRate);
    Soundsystem::Sound sound;
    sound.buffer = buffer;
    sound.loopOffset = wav.sampleHeader.loopOffset;
    SOUNDSYSTEM.sfxQueue.push_back(sound);

    /*std::ifstream& in = FILESYSTEM.getRomStream();
    in.seekg(wav.dataOffset, std::ios::beg);
    std::vector<uint8_t> data(wav.dataSize);
    in.read((char*)data.data(), wav.dataSize);
    BYTEUTILS.writeFile(data, "out.swav");*/

    std::ifstream& in = FILESYSTEM.getRomStream();
    sndType::Sseq sseq;
    SDAT.getSseq(sseq, 4);
    SSEQ.getHeader(sseq);
    in.seekg(sseq.dataOffset, std::ios::beg);
    std::vector<uint8_t> data(sseq.dataSize);
    in.read((char*)data.data(), sseq.dataSize);
    BYTEUTILS.writeFile(data, "out.sseq");

    LOG.hex("BANK:", sseq.infoEntry.bnk); // sseq 4 = bnk 119
    sndType::Bank bnk;
    SDAT.getBank(bnk, sseq.infoEntry.bnk);
    BANK.getHeader(bnk);
    BANK.parse(bnk);

    LOG.hex("BANK Total Instruments:", bnk.header.totalInstruments);
    for(size_t i = 0; i < bnk.header.totalInstruments; i++) {
        LOG.hex("BANK Record Nr.", bnk.header.records[i].fRecord);
        if(bnk.header.records[i].fRecord < 16 && bnk.header.records[i].fRecord > 0) {
            LOG.hex("Record " + std::to_string(i) + ":", std::get<sndType::Bank::RecordUnder16>(bnk.parsedInstruments[i]).swav);

        } else if(bnk.header.records[i].fRecord == 16) {

        } else if(bnk.header.records[i].fRecord == 17) {

        } else {
            LOG.err("Wrong BANK RECORD!!");
            //return 1;
        }
    }

    //return 0;

/*
    in.seekg(sseq.dataOffset + sseq.header.dataOffset, std::ios::beg);
    uint8_t byte = 0;
    //uint8_t byte = in.get();
    //if(byte == 0xC7) {// Multi Track 
    //    //...
    //}

    //LOG.hex("Message:", sseq.header.dataOffset);
    // Thanks to VgmTrans
    while(true) {
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
                    return 1;
                    break;
                
                default:
                    LOG.info("SSEQ Parser: Found weird value");
                    LOG.hex("Value:", byte);
                    return 1;
                    break;

            }
        }
    }

    return 0;
*/





    /*for(size_t i = 0; i < swar.header.totalSamples; i++) {
        LOG.info("Plaing: " + std::to_string(i));
        SWAR.getSound(swar, wav, i);
        SWAV.getSampleHeader(wav);

        buffer.clear();
        SWAV.convert(wav, buffer, 32728);

        sound.buffer = buffer;
        sound.loopOffset = wav.sampleHeader.loopOffset;
        SOUNDSYSTEM.sfxQueue.push_back(sound);

        while(!SOUNDSYSTEM.sfxQueue.empty()) {
            SDL_Delay(500);
        }
    }*/



    // Load game...

    
    
    /*Shell shell;
    shell.enter();
    return 1;*/
    /*sndType::Strm strm;
    SDAT.getStrm(strm, 38);
    STREAM.getHeader(strm);
    Soundsystem::StrmSound sound;
    sound.strm = strm;
    SOUNDSYSTEM.strmQueue.push_back(sound);*/


    /*sndType::Swar swar;
    SDAT.getSwar(swar, 0);
    SWAR.getHeader(swar);
    sndType::Swav swav;
    SWAR.getSound(swar, swav, 0);
    if(!SWAV.getSampleHeader(swav))
        LOG.err("FAILED");

    LOG.info("values: " + std::to_string(swav.sampleHeader.type));
    std::vector<uint8_t> buffer(swav.sampleHeader.length * 2);
    SWAV.convert(swav, buffer);

    Soundsystem::Sound snd;
    snd.buffer = buffer;



// LoopOffset im Header ist in 4-Byte-Einheiten, relativ zur SWAV-Datei (inkl. Header)
uint32_t rawLoopOffset = swav.sampleHeader.loopOffset * 4;

// Ziehe Header-Größe ab (0x0C = 12 Bytes), da du nur PCM-Daten im Buffer hast
if (rawLoopOffset >= 0x0C)
    rawLoopOffset -= 0x0C;
else
    rawLoopOffset = 0;

// Für IMA-ADPCM: Blockgrenze beachten (36 Byte = ein Frame)
rawLoopOffset -= rawLoopOffset % 36;

// Gültigkeit prüfen
if (rawLoopOffset < buffer.size())
    snd.loopOffset = rawLoopOffset;
else
    snd.loopOffset = 0;


    SOUNDSYSTEM.sfxQueue.push_back(snd);*/


    /*Soundsystem::StrmSound sound;
    sound.buffer = buffer;
    SOUNDSYSTEM.strmQueue.push_back()*/
    
    
    
    /*sndType::Bank bnk;
    SDAT.getBank(bnk, 28);
    BANK.getHeader(bnk);
    BANK.parse(bnk);
    LOG.info(std::to_string(SDAT.getSsarCount()));*/
    /*sndType::Swar swar;
    SDAT.getSwar(swar, 0);
    SWAR.getHeader(swar);
    Swar::Sound sound = SWAR.getSound(swar, 43);
    LOG.hex("Offset:", sound.offset);
    LOG.hex("Size  :", sound.size);*/



    /*return 0;
    LOG.info("BNK totalInstruments: " + std::to_string(bnk.header.totalInstruments));
    LOG.info("BNK items in records: " + std::to_string(bnk.header.records.size()));
    LOG.info("");
    for (size_t a = 0; a < bnk.header.records.size(); a++) {
    LOG.info("Record " + std::to_string(a));
    LOG.info("  fRecord " + std::to_string(bnk.header.records[a].fRecord));


    // ONLY LOGGING
    std::visit([a](auto& instrument) {
        using T = std::decay_t<decltype(instrument)>;

        if constexpr (std::is_same_v<T, sndType::Bank::RecordUnder16>) {
            LOG.info("    RecordUnder16");
            LOG.info("      SWAV: " + std::to_string(instrument.swav));
            LOG.info("      SWAR: " + std::to_string(instrument.swar));
            LOG.info("      Note: " + std::to_string(instrument.note));
            LOG.info("      ADSR: " + std::to_string(instrument.attack) + ", " +
                     std::to_string(instrument.decay) + ", " +
                     std::to_string(instrument.sustain) + ", " +
                     std::to_string(instrument.release));
            LOG.info("      Pan:  " + std::to_string(instrument.pan));

        } else if constexpr (std::is_same_v<T, sndType::Bank::Record16>) {
            LOG.info("    Record16");
            LOG.info("      LowNote: " + std::to_string(instrument.lowNote));
            LOG.info("      UpNote:  " + std::to_string(instrument.upNote));
            for (size_t i = 0; i < instrument.defines.size(); ++i) {
                const auto& d = instrument.defines[i];
                LOG.info("      Define[" + std::to_string(i) + "]");
                LOG.info("        SWAV: " + std::to_string(d.swav));
                LOG.info("        SWAR: " + std::to_string(d.swar));
                LOG.info("        Note: " + std::to_string(d.note));
                LOG.info("        ADSR: " + std::to_string(d.attack) + ", " +
                         std::to_string(d.decay) + ", " +
                         std::to_string(d.sustain) + ", " +
                         std::to_string(d.release));
                LOG.info("        Pan:  " + std::to_string(d.pan));
            }

        } else if constexpr (std::is_same_v<T, sndType::Bank::Record17>) {
            LOG.info("    Record17");
            for (int r = 0; r < 8; ++r) {
                LOG.info("      RegionEnd[" + std::to_string(r) + "]: " + std::to_string(instrument.regEnds[r]));
            }
            for (size_t i = 0; i < instrument.defines.size(); ++i) {
                const auto& d = instrument.defines[i];
                LOG.info("      Define[" + std::to_string(i) + "]");
                LOG.info("        SWAV: " + std::to_string(d.swav));
                LOG.info("        SWAR: " + std::to_string(d.swar));
                LOG.info("        Note: " + std::to_string(d.note));
                LOG.info("        ADSR: " + std::to_string(d.attack) + ", " +
                         std::to_string(d.decay) + ", " +
                         std::to_string(d.sustain) + ", " +
                         std::to_string(d.release));
                LOG.info("        Pan:  " + std::to_string(d.pan));
            }
        }
    }, bnk.parsedInstruments[a]);
}*/

    //SDL_Delay(1000);
    
    //LOG.info(std::to_string(SDAT.getSSARCount()));

    /*
    BNK bnk;
    SDAT.getBNK(bnk, 2);
    std::ifstream& stream = FILESYSTEM.getRomStream();
    stream.seekg(bnk.dataOffset, std::ios::beg);
    std::vector<uint8_t> buffer(bnk.dataSize);
    stream.read((char*)buffer.data(), bnk.dataSize);
    BYTEUTILS.writeFile(buffer, "outData.sbnk"); */
    
    
    /*std::vector<uint8_t> fileBuffer;
    STREAM.convert(strm, fileBuffer);
    BYTEUTILS.writeFile(fileBuffer, "sound.wav");*/

    /*SDAT.getSTRM(strm, 41);
    LOG.hex("Offset:", strm.dataOffset);
    std::vector<uint8_t> sound;
    STREAM.convert(strm, sound);


    BYTEUTILS.writeFile(sound, "out.wav");*/

    /*uint32_t offsetbefore = 0x0;
    std::vector<uint8_t> sound;
    int strmCount = SDAT.getSTRMCount() * 5;
    SOUNDSYSTEM.init();
    for(int i = 0; i < strmCount; i++) {
        SDAT.getSTRM(strm, i);
        if(strm.dataOffset == offsetbefore)
            continue;

        LOG.hex("Offset:", strm.dataOffset);
        STREAM.convert(strm, sound);
        sound.clear();
        //std::filesystem::path path = "STRM" + std::to_string(i) + ".wav";
        //BYTEUTILS.writeFile(sound, path);

        offsetbefore = strm.dataOffset;
    }*/


    
    
    size_t loops = 0;
    while (true) {
        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            }
        }

        /*if(loops == 0) {
            SDAT.getSTRM(strm, 38);
            STREAM.getHeader(strm);
            strmSound.strm = strm;
            SOUNDSYSTEM.strmQueue.push_back(strmSound);
        }*/
        /*
        if(loops > 0 && SOUNDSYSTEM.strmQueue.empty()) {
            SDAT.getSTRM(strm, 39);
            STREAM.getHeader(strm);
            strmSound.strm = strm;
            SOUNDSYSTEM.strmQueue.push_back(strmSound);
        }*/

        
        //loops++;
        SDL_Delay(500);
    }
    
    
    /*STRM strm;

    uint32_t offsetbefore;
    for (int i = 43; i < SDAT.getSTRMCount(); i++) {
        SDAT.getSTRM(strm, i);
        if(strm.dataOffset == offsetbefore)
            continue;
        //LOG.hex("Offset:", strm.dataOffset);
        LOG.info("Sound Nr: " + std::to_string(i));
        std::vector<uint8_t> sound;
        SOUNDSYSTEM.convertStrm(strm, sound);
        offsetbefore = strm.dataOffset;
    }*/
   /*cutSceneSound sound = cutSceneSound::ZELDA_FIRST_APPEAR;
   STRM strm;
   SDAT.getSTRM(strm, 43);
   std::vector<uint8_t> soundData;
   for(int i = 0; i < 1000; i++) {
        SOUNDSYSTEM.playCutsceneSound(sound);
   }
   soundData = std::move(std::vector<uint8_t>());
   SDL_Delay(5000);*/

    
    //LOG.hex("Offset:", strm.dataOffset);

    //std::ifstream& in = FILESYSTEM.getRomStream();
    

    /*
    //std::ofstream of("STRM0.strm", std::ios::binary);
    int strCount = SDAT.getSTRMCount();
    for(int count = 0; count < strCount; count++) {
        std::ofstream of(std::to_string(count), std::ios::binary);
        SDAT.getSTRM(strm, count);
        in.seekg(strm.dataOffset, std::ios::beg);
        std::vector<char> buffer(strm.dataSize);
        in.read(buffer.data(), strm.dataSize);
        of.write(buffer.data(), in.gcount());
    }
    */
    

    /*in.seekg(strm.dataOffset, std::ios::beg);
    std::vector<char> buffer(strm.dataSize);
    in.read(buffer.data(), strm.dataSize);*/
    



    
    /*SOUNDSYSTEM.loadSDAT("SoundData/final_sound_data.sdat");
    STRM strm;
    SDAT.getSTRM(strm, 0);
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(strm.dataOffset, std::ios::beg);

    LOG.info(std::to_string(strm.infoEntry.fileID));

    std::vector<char> buffer(strm.dataSize);
    romStream.read(buffer.data(), strm.dataSize);
    std::ofstream of("out.strm", std::ios::binary);
    of.write(buffer.data(), romStream.gcount());*/

    LOG.info("Finished without crashing");
    return 0;
}