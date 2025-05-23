#pragma once

#include <cstdint>
#include <unordered_map>

#include "sdat.h"
#include "types.h"
#include "audioId.h"

class Soundsystem {
private:

public:
    static Soundsystem& Instance() {
        static Soundsystem instance;
        return instance;
    }

    bool loadSDAT(std::filesystem::path file) {
        return SDAT.loadSDAT(file);
    }

    Soundsystem();
    ~Soundsystem();

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