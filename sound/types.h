#pragma once

#include <cstdint>


class SSEQ {
public:
    
    struct InfoEntry {
        uint16_t fileID = 0; // 0x0 Um herauszufinden der wievielte Eintrag im Fat das ist
        // uint16_t unknown 0x2
        uint16_t bnk = 0; // 0x4 Verweist glaub ich auf die BNK(denke mal einfach z.B. bank nr1 oder nr3)
        uint8_t vol = 0; // 0x6 Volume
        uint8_t cpr = 0; // 0x7  Channel pressure
        uint8_t ppr = 0; // 0x8 Polyphonic pressure
        uint8_t ply = 0; // 0x9 Play
        // uint16_t unknown 0xA
    } infoEntry;

    uint32_t dataOffset = 0; // Relative to SDAT File offset! NO ITS NOT ITS RELATIVE TO NDS ROM!!! STUPID *****
    uint32_t dataSize = 0;
};
    
class SSAR {
public:

    struct InfoEntry {
        uint16_t fileID = 0;
    } infoEntry;
    
    uint32_t dataOffset = 0; // Relative to SDAT File offset!
    uint32_t dataSize = 0;
};

class BANK {
public:

    struct InfoEntry {
        uint16_t fileID = 0; // 0x0 Fat file id
        // uint16_t unknown 0x2
        uint16_t swar1 = 0; // 0x4
        uint16_t swar2 = 0; // 0x6 Verweise auf SWAR Wave archive (FFFF = nicht benutzt)
        uint16_t swar3 = 0; // 0x8
        uint16_t swar4 = 0; // 0xA
    } infoEntry;

    uint32_t dataOffset = 0; // Relative to SDAT File offset!
    uint32_t dataSize = 0;
};

class SWAR {
public:

    struct InfoEntry {
        uint16_t fileID = 0; // 0x0 Fat file id
    } infoEntry;

    uint32_t dataOffset = 0; // Relative to SDAT File offset!
    uint32_t dataSize = 0;
};

class PLAYER {
public:

    struct InfoEntry {
        // uint8_t unknown 0x0
        uint32_t padding = 0; // 0x1 nur 3 Bytes! nicht 4
        // uint32_t unknown 0x4
    } infoEntry;
};

class GROUP {
public:

    struct InfoEntry {
        uint32_t itemCount = 0; // 0x1 Number of items in this group

        struct group { // array of group
            uint32_t type = 0; // 0x0700 = SEQ, 0x0803 = SEQARC(ssar), 0x0601 = BANK, 0x0402 = WAVEARC
            uint32_t nEntry = 0;
        };
        // ID/type values: 700h=SSEQ, 803h=SSAR, 601h=BANK, 402h=SWAR.
        // Index/nEntry: Entry number in the corresponding SSEQ/SSAR/BANK/SWAR list.

        std::vector<group> groups;

    } infoEntry;
};

class PLAYER2 {
public:

    struct InfoEntry {
        uint8_t nCount = 0; // 0x0 number of USED entries in array below
        // 001h 16  v[16]  ;unknown array (UNUSED entries are set to FF) (nochmal überprüfen was das ist)
    } infoEntry;
};

class STRM {
public:

    struct InfoEntry {
        uint16_t fileID = 0; // 0x0
        // uint16_t unknown 0x2
        uint8_t vol = 0; // 0x4 volume
        uint8_t pri = 0; // 0x5 priority? maybe?
        uint8_t ply = 0; // 0x6 play?
        // 5 Bytes reserved
    } infoEntry;

    struct Header {
        uint32_t Id = 0;
        uint32_t filesize = 0;
        uint8_t type = 0;
        uint8_t loop = 0;
        uint8_t channels = 0;
        uint16_t samplingRate = 0;
        uint32_t loopOffset = 0; // in Samples (decoded audio!)
        uint32_t totalSamples = 0; // (decoded audio!)
        uint32_t waveDataOffset = 0;
        uint32_t totalBlocks = 0;
        uint32_t blockLength = 0;
        uint32_t samplesBlock = 0; // Samples per block (decoded audio!)
        uint32_t lastBlockLength = 0;
        uint32_t samplesLastBlock = 0; // (decoded audio!)
        uint32_t dataSize = 0;
        //uint32_t dataOffset;
    } header;

    uint32_t dataOffset = 0; // Relative to SDAT File offset! 
    uint32_t dataSize = 0;

    std::vector<uint8_t> rawData;
};