#pragma once

#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <SDL2/SDL.h>
#include <bits/stdc++.h>
#include <mutex>

#include "sdat.h"
#include "types.h"
#include "audioId.h"

class Soundsystem {
private:
    std::ifstream audioStream;

static void mixerCallback(void* userdata, Uint8* stream, int len);

public:
    struct Sound {
        uint8_t type = 0; // 0 = STRM, 
        uint16_t loopOffset = 0;
        sndType::Strm strm;

        std::vector<uint8_t> buffer;

        size_t playPosition = 0; // Offset of reading, used by updateBuffer function
        // Function pointer to update buffer
        //void (*updateBuffer)(size_t index, int len);
    };

    struct StrmSound {
        sndType::Strm strm;

        // Buffer holds the decoded sound pieces
        std::vector<uint8_t> buffer;
        // Position (offset) of reading in STRM data(in rom)
        uint32_t blockPosition = 0; // Block position beim einlesen
    };

    // Holds every STRM to be played
    std::vector<StrmSound> strmQueue;
    // Every Sound you wan't to play needs to be pushed into this vector
    std::vector<Sound> sfxQueue;

    static Soundsystem& Instance() {
        static Soundsystem instance;
        return instance;
    }

    // Loads SDAT File(overwrites currently loaded sdat!)
    // @param file : Path to file inside of rom
    bool loadSDAT(std::filesystem::path file) {
        return SDAT.loadSDAT(file);
    }

    Soundsystem();
    ~Soundsystem();

    bool init();


    bool playMusic();
    bool playCutsceneSound(cutSceneSound ID);
    bool playSoundEffect();

    void stopMusic();
    void stopCutsceneSounds();
    void stopSoundEffects();

    // Stops every sound and music
    void stopAll();
    
    //bool playWav(std::vector<uint8_t>& wav);
};

#define SOUNDSYSTEM Soundsystem::Instance()