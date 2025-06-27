#pragma once

#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <memory>

#include "sdat.h"


// The default Samplerate of the Nintendo Ds
#define SAMPLERATE 32728

//#include "strm.h" // Würde include loop ergeben und compiler errors werfen
class Strm;

class Soundsystem {
private:
    std::ifstream audioStream;

    static void mixerCallback(void* userdata, uint8_t* stream, int len);

public:
    struct Sound {
        uint16_t loopOffset = 0;

        std::vector<uint8_t> buffer;
        uint8_t volume = 64;

        size_t playPosition = 0; // Offset of reading, used by updateBuffer function
        // Function pointer to update buffer
        //void (*updateBuffer)(size_t index, int len);
    };

    struct StrmSound {
        // Alle unique_ptr können NICHT kopiert sondern nur über std::move übertragen werden
        // ( Wenn unique_ptr in einem struct ist, muss ganzes struct gemoved werden!!)
        std::unique_ptr<Strm> strm;
        //Strm strm;

        // Buffer holds the decoded sound pieces
        std::vector<uint8_t> buffer;
        // Position (offset) of reading in STRM data(in rom)
        size_t blockPosition = 0; // Block position beim einlesen
    };

    // Holds every STRM to be played
    std::vector<StrmSound> strmQueue;
    // Every Sound you want to play needs to be pushed into this vector
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


    /*bool playMusic();
    bool playCutsceneSound(cutSceneSound ID);
    bool playSoundEffect();

    void stopMusic();
    void stopCutsceneSounds();
    void stopSoundEffects();

    // Stops every sound and music
    void stopAll();*/
    
    //bool playWav(std::vector<uint8_t>& wav);
};

#define SOUNDSYSTEM Soundsystem::Instance()