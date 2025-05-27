#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <thread>

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


    std::vector<StrmSound>& strmQueue = SOUNDSYSTEM.strmQueue;
    for(size_t index = 0; index < strmQueue.size(); index++) {
        StrmSound& sound = SOUNDSYSTEM.strmQueue[index];
        STRM::Header& header = SOUNDSYSTEM.strmQueue[index].strm.header;

        if(sound.buffer.empty() && sound.blockPosition >= sound.strm.header.totalBlocks && header.loop <= 0) {
            strmQueue.erase(strmQueue.begin() + index);
            LOG.info("Deleted sound in strmQueue. Player reached end.");
            continue;
        }

        // Wenn ende von sound dann nur rest kopieren sonnst was gebraucht wird
        int toCopy = std::min(len, static_cast<int>(sound.buffer.size()));
        
        SDL_MixAudioFormat(stream, sound.buffer.data(), AUDIO_S16LSB,
                            toCopy, sound.strm.infoEntry.vol);
        
        sound.buffer.erase(sound.buffer.begin(), sound.buffer.begin() + toCopy);

        if(sound.buffer.size() <= len && (sound.blockPosition < header.totalBlocks || header.loop >= 1)) {
            for(uint32_t size = header.blockLength; size < len; size += header.blockLength) {
                // Einmal updaten reicht meistens nicht um den buffer genügend zu füllen
                STREAM.updateBuffer(SOUNDSYSTEM.strmQueue[index], len);
            }
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
    //specs.userdata = &STREAM;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &specs, &have, 0);
    if (device == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    }
    SDL_PauseAudioDevice(device, 0); // Startet Audio-Ausgabe

    return true;
}