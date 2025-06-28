#include "bnk.h"
#include "../filesystem.h"
#include "../byteutils.h"
#include "../log.h"

#include <cstdint>
#include <vector>
#include <variant>
#include <fstream>

#define ID 0x0
#define FILESIZE 0x8
#define TOTALBLOCKS 0xE
#define TOTALINSTRUMENTS 0x38
#define OFFSET_INSTRUMENT_RECORDS 0x3C
// Danach kommt instrument data


bool Bnk::getHeader() {
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(dataOffset, std::ios::beg);
        
    // Read header ang get Information
    header.id = BYTEUTILS.getBytes(romStream, 4);

    if(header.id != 0x53424E4B) {
        LOG.errHex("Bnk::getHeader: Failed to load SBNK, wrong header ID:", header.id);
        return false;
    }

    romStream.seekg(dataOffset + FILESIZE, std::ios::beg);
    header.fileSize = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(dataOffset + TOTALBLOCKS, std::ios::beg);
    header.totalBlocks = BYTEUTILS.getLittleEndian(romStream, 2);

    romStream.seekg(dataOffset + TOTALINSTRUMENTS, std::ios::beg);
    header.totalInstruments = BYTEUTILS.getLittleEndian(romStream, 4);

    // Begin record entries -> record type, data
    for(uint32_t i = 0; i < header.totalInstruments; i++) { // every entry is 4 bytes long
        romStream.seekg(dataOffset + OFFSET_INSTRUMENT_RECORDS + i * 4, std::ios::beg);
        Header::Record record;
        record.fRecord = static_cast<uint8_t>(romStream.get());
        record.offset = BYTEUTILS.getLittleEndian(romStream, 2);
        header.records.push_back(record);
    }

    return true;
}

bool Bnk::parse() {
    if(header.id != 0x53424E4B) {
        LOG.errHex("Bnk::parse: Failed to parse SBNK, wrong header ID:", header.id);
        return false;
    }

    std::ifstream& romStream = FILESYSTEM.getRomStream();

    for(uint32_t i = 0; i < header.totalInstruments; i++) {
        Header::Record& record = header.records[i];
        romStream.seekg(dataOffset + record.offset, std::ios::beg);
        
        if(record.fRecord < 16) {
            if(record.fRecord <= 0) {
                Record0 rec0;
                parsedInstruments.push_back(rec0);
                continue;
            }

            RecordUnder16 recUnder16;

            recUnder16.swav = BYTEUTILS.getLittleEndian(romStream, 2);
            recUnder16.swar = BYTEUTILS.getLittleEndian(romStream, 2);
            recUnder16.note = romStream.get();
            recUnder16.attack = romStream.get();
            recUnder16.decay = romStream.get();
            recUnder16.sustain = romStream.get();
            recUnder16.release = romStream.get();
            recUnder16.pan = romStream.get();
            parsedInstruments.push_back(recUnder16);

        } else if(record.fRecord == 16) {
            Record16 rec16;

            rec16.lowNote = romStream.get();
            rec16.upNote = romStream.get();
            size_t defineCount = rec16.upNote - rec16.lowNote + 1;
            for(size_t count = 0; count < defineCount; count++) {
                NoteDefine define;
                romStream.ignore(2); // 2 Bytes unknown // usually == 01 00 | 0x0001
                define.swav = BYTEUTILS.getLittleEndian(romStream, 2);
                define.swar = BYTEUTILS.getLittleEndian(romStream, 2);
                define.note = romStream.get();
                define.attack = romStream.get();
                define.decay = romStream.get();
                define.sustain = romStream.get();
                define.release = romStream.get();
                define.pan = romStream.get();
                rec16.defines.push_back(define);
            }

            parsedInstruments.push_back(rec16);

        } else if(record.fRecord == 17) {
            Record17 rec17;

            uint8_t regionCount = 0;
            for(uint8_t count = 0; count < 8; count++) { // 8 Einträge(je 1 bit) lesen
                uint8_t region = romStream.get();
                rec17.regEnds[count] = region;
                if(region != 0)
                    regionCount++;
            }

            for(uint8_t count = 0; count < regionCount; count++) {
                if(rec17.regEnds[count] != 0) {
                    NoteDefine define;
                    romStream.ignore(2);
                    define.swav = BYTEUTILS.getLittleEndian(romStream, 2);
                    define.swar = BYTEUTILS.getLittleEndian(romStream, 2);
                    define.note = romStream.get();
                    define.attack = romStream.get();
                    define.decay = romStream.get();
                    define.sustain = romStream.get();
                    define.release = romStream.get();
                    define.pan = romStream.get();
                    rec17.defines.push_back(define);
                }
            }
            
            parsedInstruments.push_back(rec17);

        } else {
            // When there is bullshit something must be wrong...
            LOG.err("Bank::parse: Weird value in fRecord, aborting");
            return false;
        }
        
    }

    return true;
}