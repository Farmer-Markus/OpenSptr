#pragma once

#include <cstdint>
#include <variant>
#include <vector>

// Holds all file types of contained in sdat
namespace sndType {

    class Sseq {
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

        struct Header {
            uint32_t id = 0;
            //uint16_t byteOrder = 0;
            //uint16_t version = 0;
            uint32_t fileSize = 0;
            //uint16_t headerSize = 0; // usually 0x10
            //uint16_t totalBlocks = 0; // (usually 1 = DATA)
            //uint32_t dataId = 0; // Subheader "DATA"
            //uint32_t fileSizeMinusTen = 0;
            uint32_t dataOffset = 0; // relative to sseq beginning
            // Sequenced data arrays ...
        } header;

        uint32_t dataOffset = 0; // Relative to SDAT File offset! NO ITS NOT ITS RELATIVE TO NDS ROM!!! STUPID *****
        uint32_t dataSize = 0;
    };
        
    class Ssar {
    public:

        struct InfoEntry {
            uint16_t fileID = 0;
        } infoEntry;
        
        uint32_t dataOffset = 0; // Relative to SDAT File offset!
        uint32_t dataSize = 0;
    };

    class Bank {
    public:
        // Record0 ist unnötig (glaub ich)
        struct Record0 {};
        
        struct RecordUnder16 { // fRecord < 16, the record is a note/wave definition
            uint16_t swav = 0; // the swav used | // Swav in swar
            uint16_t swar = 0; // the swar used (see Info Block --> "BANK Info Entry") 
            uint8_t note = 0; // 0..127
            uint8_t attack = 0; // 0..127
            uint8_t decay = 0; // 0..127
            uint8_t sustain = 0; // 0..127
            uint8_t release = 0; // 0..127
            uint8_t pan = 0; // 0..127, 64 = middle
        };

        struct Record16 { // fRecord = 16, the record is a range of note/wave definitions
            uint8_t lowNote = 0;
            uint8_t upNote = 0;
            
            struct NoteDefine {
                // 2 Bytes unknown // usually == 01 00 | 0x0001
                uint16_t swav = 0; // Swav in swar
                uint16_t swar = 0;
                uint8_t note = 0;
                uint8_t attack = 0;
                uint8_t decay = 0;
                uint8_t sustain = 0;
                uint8_t release = 0;
                uint8_t pan = 0;
            };

            // (upNote - lowNote + 1) * defines
            std::vector<NoteDefine> defines;
        };

        struct Record17 { // fRecord = 17, the record is a regional wave/note definition
            uint8_t regEnds[8] = {0};
            
            //uint8_t end1Reg = 0; // eg. 25  = notes 0..25
            //uint8_t end2Reg = 0; // eg. 35  = notes 26..35
            //uint8_t end3Reg = 0; // eg. 45  = notes 36..45
            //uint8_t end4Reg = 0; // eg. 55  = notes 46..55
            //uint8_t end5Reg = 0; // eg. 65  = notes 56..65
            //uint8_t end6Reg = 0; // eg. 127 = notes 66..last
            //uint8_t end7Reg = 0; // eg. 0   = none
            //uint8_t end8Reg = 0; // eg. 0   = none

            struct NoteDefine {
                // 2 Bytes unknown // usually == 01 00 | 0x0001
                uint16_t swav = 0;
                uint16_t swar = 0;
                uint8_t note = 0;
                uint8_t attack = 0;
                uint8_t decay = 0;
                uint8_t sustain = 0;
                uint8_t release = 0;
                uint8_t pan = 0;
            };

            std::vector<NoteDefine> defines;
        };

    
        struct InfoEntry {
            uint16_t fileID = 0; // 0x0 Fat file id
            // uint16_t unknown 0x2
            uint16_t swar1 = 0; // 0x4
            uint16_t swar2 = 0; // 0x6 Verweise auf SWAR Wave archive (FFFF = nicht benutzt)
            uint16_t swar3 = 0; // 0x8
            uint16_t swar4 = 0; // 0xA
        } infoEntry;

        struct Header { 
            uint32_t id = 0;
            //uint16_t byteOrder = 0;
            //uint16_t version = 0;
            uint32_t fileSize = 0;
            //uint16_t headerSize = 0;
            uint16_t totalBlocks = 0;
            //uint32_t fileSizeMinus10 = 0;
            //20 bytes reserved
            uint32_t totalInstruments = 0; // (swav'S)
            
            struct Record { // 4 bytes long
                uint8_t fRecord = 0; // can be either 0, 1..4, 16 or 17
                uint16_t offset = 0; // absolute offset of data in sbnk file to instrument data
                // 1 byte reserved
            };
            
            std::vector<Record> records; // Records (4 bytes each)
            // ... Instrument data

        } header;

        std::vector<std::variant<Record0, RecordUnder16, Record16, Record17>> parsedInstruments;

        uint32_t dataOffset = 0; // Relative to SDAT File offset! // offset where header begins
        uint32_t dataSize = 0;
    };

    class Swar {
    public:

        struct InfoEntry {
            uint16_t fileID = 0; // 0x0 Fat file id
        } infoEntry;

        struct Header {
            uint32_t id = 0; // 0x0
            // uint16_t byteorder
            // uint16_t version
            uint32_t fileSize = 0; // 0x8
            // uint16_t headerSize
            // uint16_t totalBlocks (usually 1 = DATA)
            // uint32_t DataID = Data block
            // uint32_t filesize - 0x10
            // 0x20 reserved
            uint32_t totalSamples = 0; // Number of SWAV sample blocks 0x38
            std::vector<uint32_t> sampleOffsets; // Offets to sample blocks. offset angabe 4 bytes lang(uint32_t) und so viele offsets wie totalSamples(offsets vom start der swar datei aus!)
            // Sample blocks ... starting with Type (0=PCM8, 1=PCM16, 2=IMA-ADPCM)
        } header;

        uint32_t dataOffset = 0; // Relative to SDAT File offset!
        uint32_t dataSize = 0;
    };

    class Player {
    public:

        struct InfoEntry {
            // uint8_t unknown 0x0
            uint32_t padding = 0; // 0x1 nur 3 Bytes! nicht 4
            // uint32_t unknown 0x4
        } infoEntry;
    };

    class Group {
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

    class Player2 {
    public:

        struct InfoEntry {
            uint8_t nCount = 0; // 0x0 number of USED entries in array below
            // 001h 16  v[16]  ;unknown array (UNUSED entries are set to FF) (nochmal überprüfen was das ist)
        } infoEntry;
    };

    class Strm {
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
            uint32_t id = 0;
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

        std::vector<uint8_t> rawData; // Used for audio playback
    };


    // Not directly in sdat:
    class Swav {
    public:
        struct Header { // not used
            uint32_t id = 0;
            // uint16_t byteorder
            // uint16_t version
            uint32_t filesize = 0;
            // uint16_t headersize
            uint16_t totalBlocks = 0;
            // uint32_t id DATA block
            // uint32_t filesize -10
            // Sample blocks ...
        } header;

        struct SampleHeader {
            uint8_t type = 0;
            uint8_t loop = 0;
            uint16_t samplingRate = 0;
            // uint16_t time
            uint16_t loopOffset = 0;
            uint32_t nonLoopLength = 0; // *4 nehmen um byte länge zu bekommen
            // Data ... samples (with 32bit header in case of ADPCM)

            //uint32_t totalBlocks = 0; // Custom entry(is calculated)
        } sampleHeader;

        uint32_t dataOffset = 0;
        uint32_t dataSize = 0;
    };
}