#pragma once

#include <cstdint>
#include <vector>

class Pcm {
private:

public:
    static Pcm& Instance() {
        static Pcm instance;
        return instance;
    }


    // Decode Ima-Adpcm block to Pcm16
    // @param blockData: The Blockdata
    // @param pcmData: The output Buffer to write decoded pcm data
    // @param channels: Channels of audio block. eg. mono = 1, stereo = 2
    // @param side: If stereo then this is which side is decoded(left = 0, right = 1)
    // @param ignoredSamples: Samples to be skipped/igored eg. in loops
    bool decodeImaAdpcm(const std::vector<uint8_t>& blockData, std::vector<int16_t>& pcmData,
                        int channels, int side, size_t ignoredSamples, bool hasBlockHeader);

    // Convert block PCM8 to PCM16 (NOT TESTED!)
    // @param blockData: The Blockdata
    // @param pcmData: The output Buffer to write decoded pcm data
    // @param channels: Channels of audio block. eg. mono = 1, stereo = 2
    // @param side: If stereo then this is which side is decoded(left = 0, right = 1)
    // @param ignoredSamples: Samples to be skipped/igored eg. in loops
    bool convertPcm8ToPcm16(const std::vector<uint8_t>& blockData, std::vector<int16_t>& pcmData,
                            int channels, int side, size_t ignoredSamples);

    // Interleave PCM16 to stereo (NOT TESTED!)
    // @param blockData: The Blockdata
    // @param pcmData: The output Buffer to write decoded pcm data
    // @param channels: Channels of audio block. eg. mono = 1, stereo = 2
    // @param side: If stereo then this is which side is decoded(left = 0, right = 1)
    // @param ignoredSamples: Samples to be skipped/igored eg. in loops
    bool interleavePcm16(const std::vector<int16_t>& blockData, std::vector<int16_t>& pcmData,
                        int channels, int side, size_t ignoredSamples);
    
    // Interpolate/pitch PCM16 sound/song to requested samplerate
    // IMPORTANT! The sndData vector is empty after running this function!
    // @param sndData: Sound/song data
    // @param outData: Sound/song output buffer
    // @param sndSamplerate: Samplerate of given sound/song
    // @param outSamplerate: Requested samplerate the output sound/song should have
    bool pitchInterpolatePcm16(const std::vector<int16_t>& sndData, std::vector<int16_t>& outData,
                                uint16_t sndSamplerate, uint32_t outSamplerate, int8_t semitonePitch);
};

#define PCM Pcm::Instance()