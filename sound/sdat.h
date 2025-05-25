#pragma once

#include <fstream>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "../filesystem.h"
#include "types.h"


class Sdat {
private:
    friend class Soundsystem;

    Filesystem::File sdat;

    // Offsets in relation to rom file!
    struct Sdatheader {
        uint32_t ID = 0;
        
        uint32_t infoOffset = 0;
        uint32_t infoSize = 0;

        uint32_t fatOffset = 0;
        uint32_t fatSize = 0;

        uint32_t fileOffset = 0; // File Block with actual Data
        uint32_t fileSize = 0;
    } sdatheader;

public:
    static Sdat& Instance() {
        static Sdat instance;
        return instance;
    }



    // INFO Sektion im SDAT verweißt auf Listen für jeden DateiTyp und auch Player,Player2 wofür auch immer das ist
    // Die Listen haben dann wiederum Infos darüber wieviele Dateien dieses Types in der Liste vorhanden sind und
    // danach kommen die ganzen offsets für diese Dateien(offsets -> FAT)
    struct SDATInfoEntry {
        struct infoList {
            uint32_t entryCount = 0;
            std::vector<uint32_t> entries; // Holds offsets for entries
        };
                
        infoList sseq;      // 0x8
        infoList ssar;      // 0xC
        infoList bank;      // 0x10
        infoList swar;      // 0x14
        infoList player;    // 0x18
        infoList group;     // 0x1C
        infoList player2;   // 0x20
        infoList strm;      // 0x24
    } sdatInfoEntry;


    ~Sdat();

    bool loadSDAT(std::filesystem::path file);

    // Die countteste "" Datei zurück geben. Returns all 0 when file out of Range(provided number higher than actual files)
    void getSSEQ(SSEQ& sseq, int count);
    void getSSAR(SSAR& ssar, int count);
    void getBANK(BANK& bank, int count);
    void getSWAR(SWAR& swar, int count);
    void getPLAYER(PLAYER& player, int count);
    void getGROUP(GROUP& group, int count);
    void getPLAYER2(PLAYER2& player2, int count);
    void getSTRM(STRM& strm, int count);


    int getSSEQCount() {return sdatInfoEntry.sseq.entryCount;}
    int getSSARCount() {return sdatInfoEntry.ssar.entryCount;}
    int getBANKCount() {return sdatInfoEntry.bank.entryCount;}
    int getSWARCount() {return sdatInfoEntry.swar.entryCount;}
    int getPLAYERCount() {return sdatInfoEntry.player.entryCount;}
    int getGROUPCount() {return sdatInfoEntry.group.entryCount;}
    int getPLAYER2Count() {return sdatInfoEntry.player2.entryCount;}
    int getSTRMCount() {return sdatInfoEntry.strm.entryCount;}

};

#define SDAT Sdat::Instance()