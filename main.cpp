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


int main(int argc, char* argv[]) {
    SOUNDSYSTEM.loadSDAT("SoundData/final_sound_data.sdat");
    
    STRM strm;
    /*SDAT.getSTRM(strm, 41);
    LOG.hex("Offset:", strm.dataOffset);
    std::vector<uint8_t> sound;
    STREAM.convert(strm, sound);


    BYTEUTILS.writeFile(sound, "out.wav");*/

    uint32_t offsetbefore = 0x0;
    std::vector<uint8_t> sound;
    int strmCount = SDAT.getSTRMCount() * 5;
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
    }

    SDL_Delay(5000);
    
    
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


    return 0;
}