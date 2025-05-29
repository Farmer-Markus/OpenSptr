#include <iostream>
#include <filesystem>

#include <fstream>
#include <ostream>
#include <vector>

#include <SDL2/SDL.h>

#include "log.h"

#include "sound/sound.h"
#include "sound/types.h"
#include "byteutils.h"
#include "sound/audioId.h"
#include "sound/stream.h"
#include "sound/bank.h"
#include "sound/swar.h"

//using namespace sndType;


int main(int argc, char* argv[]) {
    SOUNDSYSTEM.loadSDAT("SoundData/final_sound_data.sdat");
    SOUNDSYSTEM.init();
    
    sndType::Strm strm;
    SDAT.getStrm(strm, 38);
    STREAM.getHeader(strm);
    Soundsystem::StrmSound sound;
    sound.strm = strm;
    SOUNDSYSTEM.strmQueue.push_back(sound);


    
    //Filesystem::File file = FILESYSTEM.getFile("English/Message");
    //LOG.hex("File Offset:", file.offset);
    
    
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

        
        loops++;
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