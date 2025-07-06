#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <thread>

#include <SDL2/SDL.h>

#include "sound.h"
#include "strm.h"
#include "pcm.h"
#include "../byteutils.h"
#include "../filesystem.h"
#include "../log.h"

Soundsystem::Soundsystem() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        LOG.SDLerr("Soundsystem: Failed initializing SDL2 audio", SDL_GetError());
        return;
    }

    LOG.debug("Soundsystem: SDL2 Audio initialized successfully.");
}

Soundsystem::~Soundsystem() {
    SDL_AudioQuit();
    audioStream.close();
}

// Called by SDL when audio buffer empty. Need to fill new audio infos into buffer.
void Soundsystem::mixerCallback(void* userdata, uint8_t* stream, int len) {
    memset(stream, 0, len);


    std::vector<StrmSound>& strmQueue = SOUNDSYSTEM.strmQueue;
    for(size_t index = 0; index < strmQueue.size(); index++) {
        StrmSound& sound = SOUNDSYSTEM.strmQueue[index];
        Strm::Header& header = SOUNDSYSTEM.strmQueue[index].strm.header;

        if(sound.buffer.empty() && sound.blockPosition >= header.totalBlocks /*&& header.loop <= 0*/) {
            strmQueue.erase(strmQueue.begin() + index);
            LOG.debug("Soundsystem::mixerCallback: Deleted sound in strmQueue. Player reached end.");
            continue;
        }

        // Wenn ende von sound dann nur rest kopieren sonnst was gebraucht wird
        int toCopy = std::min(len, static_cast<int>(sound.buffer.size()));
        
        SDL_MixAudioFormat(stream, sound.buffer.data(), AUDIO_S16LSB,
                            toCopy, sound.strm.infoEntry.vol);
        
        sound.buffer.erase(sound.buffer.begin(), sound.buffer.begin() + toCopy);

        if(sound.buffer.size() <= len) {
            for(uint32_t size = header.blockLength; size < len && sound.blockPosition < header.totalBlocks; size += header.blockLength) {
                /*if(sound.blockPosition >= header.totalBlocks) {
                    sound.blockPosition = header.loopOffset * header.samplesBlock;
                    LOG.info("STRM: Looping");
                }*/
                
                // Einmal updaten reicht meistens nicht um den buffer genügend zu füllen
                // NOCH SCHÖNER MACHEN!!! <------------------------------------------------------------------------>
                if(!sound.strm.updateBuffer(sound.buffer, sound.blockPosition, SAMPLERATE))
                //if(!Strm::updateBuffer(SOUNDSYSTEM.strmQueue[index], len, SAMPLERATE))
                    strmQueue.erase(strmQueue.begin() + index);
                    index--;
            }
        }
    }

    /*std::vector<Sound>& sfxQueue = SOUNDSYSTEM.sfxQueue;
    for(size_t index = 0; index < sfxQueue.size(); index++) {
        Sound& sound = SOUNDSYSTEM.sfxQueue[index];

        if(sound.playPosition >= sound.buffer.size()) {
            sfxQueue.erase(sfxQueue.begin() + index);
            //sound.playPosition = sound.loopOffset;
            continue;
        }

        int tmp = sound.buffer.size() - sound.playPosition;
        // Wenn ende von sound dann nur rest kopieren sonnst was gebraucht wird
        int toCopy = std::min(len, tmp);
        
        SDL_MixAudioFormat(stream, sound.buffer.data() + sound.playPosition, AUDIO_S16LSB,
                            toCopy, sound.volume); //128 is max vol
        
        sound.playPosition += toCopy;
        //LOG.info("Tocopy: " + std::to_string(toCopy));
    }*/

    for(size_t index = 0; index < SOUNDSYSTEM.sseqQueue.size(); index++) {
        if(SOUNDSYSTEM.sseqQueue[index] == nullptr) {
            SOUNDSYSTEM.sseqQueue.erase(SOUNDSYSTEM.sseqQueue.begin() + index);
            index--;
            continue;
        }
        Sequencer& seq = *SOUNDSYSTEM.sseqQueue[index];

        for(auto& [nr, track] : seq.tracks) {
            if(track.activeNotes.empty())
                continue;

            uint8_t frecord = track.program.frecord;
            if(!frecord) {
                continue;
            }
    
            if(frecord < 16 && frecord > 0) {
                //continue;
                LOG.debug("MIXER: Mixing FRECORD-UNDER-16 Event!");
                
                Bnk::RecordUnder16& record = std::get<Bnk::RecordUnder16>(track.program.record);
                Swav& swav = seq.instruments[{record.swar, record.swav}];

                for(size_t nCount = 0; nCount < track.activeNotes.size(); nCount++) {
                    Sequencer::Note& note = track.activeNotes[nCount];
                    if(note.sndData.empty()) {
                        std::vector<int16_t> pcmData;
                        float pitch = note.absKey - record.note;

                        if(swav.sampleHeader.loop > 0) {
                            float pitchFactor = std::pow(2.0f, pitch / 12.0f);
                            note.loopOffset = static_cast<size_t>((swav.sampleHeader.loopOffset * 4) / pitchFactor);
                        }

                        PCM.pitchInterpolatePcm16(swav.soundData, pcmData, swav.sampleHeader.samplingRate,
                                                    SAMPLERATE, pitch);

                        std::vector<int16_t> stereoPcmData(2 * pcmData.size());
                        for(size_t i = 0; i < pcmData.size(); i++) {
                            stereoPcmData[2 * i] = pcmData[i];
                            stereoPcmData[2 * i + 1] = pcmData[i];
                        }
                        size_t stereoPcmDataSize = stereoPcmData.size();
                        note.sndData.resize(stereoPcmDataSize * sizeof(int16_t));
                        std::memcpy(note.sndData.data(), stereoPcmData.data(), stereoPcmDataSize * sizeof(int16_t));

                    }
                    
                    size_t dataSize = note.sndData.size();
                    if(note.playPosition >= dataSize) {
                        if(swav.sampleHeader.loop) {
                            note.playPosition = note.loopOffset;
                        } else {
                            LOG.info("MIXER: Note not loopable!");
                            track.activeNotes.erase(track.activeNotes.begin() + nCount);
                            nCount--;
                            continue;
                        }
                    }

                    size_t cursor = dataSize - note.playPosition;
                    size_t toCopy = std::min(len, static_cast<int>(cursor));

                    SDL_MixAudioFormat(stream, note.sndData.data() + note.playPosition, AUDIO_S16LSB,
                                        toCopy, track.vol);

                    note.playPosition += toCopy;
                    
                }


            } else if(frecord == 16) {
                /*LOG.debug("MIXER: Mixing FRECORD-16 Event!");

                Bnk::Record16& record = std::get<Bnk::Record16>(track.program.record);
                for(size_t nCount = 0; nCount < track.activeNotes.size(); nCount++) {
                    Sequencer::Note& note = track.activeNotes[nCount];
                    Bnk::NoteDefine& define = record.defines[note.absKey];
                    Swav& swav = seq.instruments[{define.swar, define.swav}];

                    if(note.sndData.empty()) {
                        std::vector<int16_t> pcmData;

                        float pitch = note.absKey - define.note;
                        float pitchFactor = std::pow(2.0f, pitch / 12.0f);
                        note.loopOffset = static_cast<size_t>((swav.sampleHeader.loopOffset * 4) / pitchFactor);

                        PCM.pitchInterpolatePcm16(swav.soundData, pcmData, swav.sampleHeader.samplingRate,
                                                    SAMPLERATE, pitch);
                        
                        std::vector<int16_t> stereoPcmData(2 * pcmData.size());
                        for(size_t i = 0; i < pcmData.size(); i++) {
                            stereoPcmData[2 * i] = pcmData[i];
                            stereoPcmData[2 * i + 1] = pcmData[i];
                        }
                        size_t stereoPcmDataSize = stereoPcmData.size();
                        note.sndData.resize(stereoPcmDataSize * sizeof(int16_t));
                        std::memcpy(note.sndData.data(), stereoPcmData.data(), stereoPcmDataSize * sizeof(int16_t));

                    }

                    size_t dataSize = note.sndData.size();
                    if(note.playPosition >= dataSize) {
                        if(swav.sampleHeader.loop) {
                            note.playPosition = note.loopOffset;
                        } else {
                            LOG.info("MIXER: Note not loopable!");
                            track.activeNotes.erase(track.activeNotes.begin() + nCount);
                        }
                    }

                    size_t cursor = dataSize - note.playPosition;
                    size_t toCopy = std::min(len, static_cast<int>(cursor));

                    SDL_MixAudioFormat(stream, note.sndData.data() + note.playPosition, AUDIO_S16LSB,
                                        toCopy, track.vol);

                    note.playPosition += toCopy;

                }*/
                
            } else {
                LOG.debug("MIXER: Mixing FRECORD-17 Event!");
                Bnk::Record17& record = std::get<Bnk::Record17>(track.program.record);

                for(size_t nCount = 0; nCount < track.activeNotes.size(); nCount++) {
                    Sequencer::Note& note = track.activeNotes[nCount];

                    uint8_t region = 0;
                    for(; region < 8; region++) {
                        if(record.regEnds[region] == 0)
                            break;
                                                
                        if(note.absKey <= record.regEnds[region])
                            break;
                    }
                    // region holds region for this note
                    if(region >= record.defines.size()) {
                        LOG.err("MIXER: Undefined region");
                        continue;
                    }

                    //----------------------------------------------------------------------
                    Bnk::NoteDefine& define = record.defines[region];
                    Swav& swav = seq.instruments[{define.swar, define.swav}];

                    if(note.sndData.empty()) {
                        std::vector<int16_t> pcmData;

                        float pitch = note.absKey - define.note;
                        float pitchFactor = std::pow(2.0f, pitch / 12.0f);
                        note.loopOffset = static_cast<size_t>((swav.sampleHeader.loopOffset * 4) / pitchFactor);

                        PCM.pitchInterpolatePcm16(swav.soundData, pcmData, swav.sampleHeader.samplingRate,
                                                    SAMPLERATE, pitch);
                        
                        std::vector<int16_t> stereoPcmData(2 * pcmData.size());
                        for(size_t i = 0; i < pcmData.size(); i++) {
                            stereoPcmData[2 * i] = pcmData[i];
                            stereoPcmData[2 * i + 1] = pcmData[i];
                        }
                        size_t stereoPcmDataSize = stereoPcmData.size();
                        note.sndData.resize(stereoPcmDataSize * sizeof(int16_t));
                        std::memcpy(note.sndData.data(), stereoPcmData.data(), stereoPcmDataSize * sizeof(int16_t));

                    }

                    size_t dataSize = note.sndData.size();
                    if(note.playPosition >= dataSize) {
                        if(swav.sampleHeader.loop) {
                            note.playPosition = note.loopOffset;
                        } else {
                            LOG.info("MIXER: Note not loopable!");
                            track.activeNotes.erase(track.activeNotes.begin() + nCount);
                            nCount--;
                            continue;
                        }
                    }

                    size_t cursor = dataSize - note.playPosition;
                    size_t toCopy = std::min(len, static_cast<int>(cursor));

                    SDL_MixAudioFormat(stream, note.sndData.data() + note.playPosition, AUDIO_S16LSB,
                                        toCopy, track.vol);

                    note.playPosition += toCopy;


                }
            }
        }
    }

}

bool Soundsystem::init() {
    FILESYSTEM.newRomStream(audioStream);
    
    SDL_AudioSpec specs, have;
    SDL_zero(specs);
    specs.freq = SAMPLERATE;
    specs.format = AUDIO_S16LSB;
    specs.channels = 2;
    specs.samples = 1024; //4096 //2048
    specs.callback = mixerCallback;
    //specs.userdata = &STREAM;

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &specs, &have, 0);
    if (device == 0) {
        // Noch eigene LogFunktion benutzen lassen !!
        LOG.SDLerr("Failed to open audio:", SDL_GetError());
        return false;
    }
    SDL_PauseAudioDevice(device, 0); // Startet Audio-Ausgabe

    LOG.debug("Soundsystem::init: Audio device started successfully.");
    return true;
}


/*for(Sequencer::Note& note : track.activeNotes) {
                    //---------------------
                    std::vector<uint8_t> dataBuffer;
                    std::vector<int16_t> pcmData;
                    std::vector<int16_t> stereoPcmData;
                    PCM.pitchInterpolatePcm16(swav.soundData, pcmData, swav.sampleHeader.samplingRate,
                                                SAMPLERATE, 0);
                
                    stereoPcmData.resize(2 * pcmData.size());
                    for(size_t i = 0; i < pcmData.size(); i++) {
                        stereoPcmData[2 * i] = pcmData[i];
                        stereoPcmData[2 * i + 1] = pcmData[i];
                    }

                    size_t outBufferSize = dataBuffer.size();
                    size_t pcmDataSize = stereoPcmData.size();
                    dataBuffer.resize(outBufferSize + pcmDataSize * sizeof(int16_t));
                    std::memcpy(dataBuffer.data() + outBufferSize, stereoPcmData.data(), pcmDataSize * sizeof(int16_t));
                    //---------------------

                    size_t dataSize = swav.soundData.size();
                    if(note.playPosition >= dataSize) {
                        note.playPosition = swav.sampleHeader.loopOffset;
                    }


                    size_t cursor = dataSize - note.playPosition;
                    size_t toCopy = std::min(len, static_cast<int>(cursor));

                    SDL_MixAudioFormat(stream, dataBuffer.data() + note.playPosition, AUDIO_S16LSB,
                                        toCopy, track.vol);
                    LOG.info("Mixing");

                    note.playPosition += toCopy;
                }*/




/*size_t dataSize = note.sndData.size();
                    size_t remaining = static_cast<size_t>(len);
                    uint8_t* streamPtr = stream + (len - remaining);

                    while (remaining > 0) {
                        if (note.playPosition >= dataSize) {
                            note.playPosition = note.loopOffset;
                        }

                        size_t cursor = dataSize - note.playPosition;
                        size_t toCopy = std::min(remaining, cursor);

                        SDL_MixAudioFormat(streamPtr, note.sndData.data() + note.playPosition, AUDIO_S16LSB,
                                        toCopy, track.vol);

                        note.playPosition += toCopy;
                        streamPtr += toCopy;
                        remaining -= toCopy;
                    }*/