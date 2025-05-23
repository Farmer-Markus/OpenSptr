#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cstring>

#include <SDL2/SDL.h>

#include "sound.h"
#include "types.h"
#include "../byteutils.h"
#include "../filesystem.h"
#include "../log.h"

Soundsystem::Soundsystem() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        LOG.SDLerr("Failed initializing SDL2 Mixer:", SDL_GetError());
        return;
    }
}

Soundsystem::~Soundsystem() {
    SDL_AudioQuit();
}


bool Soundsystem::playCutsceneSound(cutSceneSound ID) {
    /*STRM strm;
    SDAT.getSTRM(strm, ID);
    std::vector<uint8_t> soundData;
    if(!convertStrm(strm, soundData)) {
        LOG.err("Failed to play cutscene sound, convertStrm returned false!");
        return false;
    }

    if(!SDL_WasInit(SDL_INIT_AUDIO)) {
        LOG.err("Cannot play sound! Sdl mixer not initialized.");
        return false;
    }*/

    //SDL_AudioSpec spec;


    /*
    // Load WAV file from a file (for demonstration)
    SDL_RWops* rw = SDL_RWFromMem(soundData.data(), soundData.size());
    Mix_Chunk* sound = Mix_LoadWAV_RW(rw, 1);
    if (sound == NULL) {
        LOG.SDLerr("Failed loading converted STRM:", Mix_GetError());
        return false;
    }

    Mix_ChannelFinished(closeAudio);

    int channel = Mix_PlayChannel(-1, sound, 0);
    playingSounds[channel] = sound;

    soundData.clear();
    soundData.shrink_to_fit();
    */
    return true;
}