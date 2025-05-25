#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cstring>

#include <SDL2/SDL.h>

#include "sound.h"
#include "types.h"
#include "stream.h"
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
    audioStream.close();
}


// HIER WEITER ARBEITEN!!
// Called by SDL when audio buffer empty. Need to fill new audio infos into buffer.
void Soundsystem::mixerCallback(void* userdata, Uint8* stream, int len) {
    memset(stream, 0, len);

    // Mix STRM's
    std::vector<StrmSound>& strmQueue = SOUNDSYSTEM.strmQueue;
    for(size_t index = 0; index < strmQueue.size(); index++) {
        if(strmQueue[index].buffer.empty() && strmQueue[index].blockPosition >= strmQueue[index].strm.header.totalBlocks) {
            strmQueue.erase(strmQueue.begin() + index);
            LOG.info("Deleted sound in strmQueue. Player reached end.");
            continue;
        }
        
        // Wenn ende von sound dann nur rest kopieren sonnst was gebraucht wird
        int toCopy = std::min(len, static_cast<int>(strmQueue[index].buffer.size()));

        SDL_MixAudioFormat(stream, strmQueue[index].buffer.data(), AUDIO_S16LSB,
                            toCopy, strmQueue[index].strm.infoEntry.vol);
        
        strmQueue[index].buffer.erase(strmQueue[index].buffer.begin(), 
                                        strmQueue[index].buffer.begin() + toCopy);
        
        if(strmQueue[index].buffer.size() <= len && strmQueue[index].blockPosition < strmQueue[index].strm.header.totalBlocks) {
            // Trigger function to fill buffer
            STREAM.updateBuffer(strmQueue[index], len);
        }
    }

}

bool Soundsystem::init() {
    audioStream.open("game.nds", std::ios::binary);
    
    SDL_AudioSpec specs, have;
    SDL_zero(specs);
    specs.freq = 32728; // 44100
    specs.format = AUDIO_S16SYS;
    specs.channels = 2;
    specs.samples = 1024;
    specs.callback = mixerCallback;
    specs.userdata = &STREAM;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &specs, &have, 0);
    if (device == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    }
    SDL_PauseAudioDevice(device, 0); // Startet Audio-Ausgabe

    return true;
}