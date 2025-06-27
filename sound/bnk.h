#pragma once

#include <cstdint>
#include <vector>
#include <variant>


class Bnk {
private:

public:
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


        std::vector<NoteDefine> defines;
    };


    struct InfoEntry {
        uint16_t fileID = 0; // 0x0 Fat file id
        // uint16_t unknown 0x2
        uint16_t swar[4] = {0};
        /*uint16_t swar1 = 0; // 0x4
        uint16_t swar2 = 0; // 0x6 Verweise auf SWAR Wave archive (FFFF = nicht benutzt)
        uint16_t swar3 = 0; // 0x8
        uint16_t swar4 = 0; // 0xA*/
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

    bool getHeader();
    bool parse();
};