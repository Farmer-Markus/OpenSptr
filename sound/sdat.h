#pragma once

#include <fstream>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "strm.h"
#include "sseq.h"
#include "bnk.h"
#include "swar.h"
//#include "strm.h"
#include "../filesystem.h"


class Sdat {
private:
    friend class Soundsystem;

    Filesystem::File sdat;

    // Offsets in relation to rom file!
    struct Header {
        uint32_t id = 0;
        
        uint32_t infoOffset = 0;
        uint32_t infoSize = 0;

        uint32_t fatOffset = 0;
        uint32_t fatSize = 0;

        uint32_t fileOffset = 0; // File Block with actual Data
        uint32_t fileSize = 0;
    } header;

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
        infoList bnk;      // 0x10
        infoList swar;      // 0x14
        infoList player;    // 0x18
        infoList group;     // 0x1C
        infoList player2;   // 0x20
        infoList strm;      // 0x24
    } sdatInfoEntry;


    bool loadSDAT(std::filesystem::path path);

    // Die countteste "" Datei zurück geben. Returns all 0 when file out of Range(provided number higher than actual files)
    bool getSseq(Sseq& sseq, int count);
    //bool getSsar(Ssar& ssar, int count);
    bool getBnk(Bnk& bnk, int count);
    bool getSwar(Swar& swar, uint16_t count);
    //bool getPlayer(Player& player, int count);
    //bool getGroup(Group& group, int count);
    //bool getPlayer2(Player2& player2, int count);
    bool getStrm(Strm& strm, int count);


    int getSseqCount()  {return sdatInfoEntry.sseq.entryCount;}
    int getSsarCount() {return sdatInfoEntry.ssar.entryCount;}
    int getBankCount() {return sdatInfoEntry.bnk.entryCount;}
    int getSwarCount() {return sdatInfoEntry.swar.entryCount;}
    int getPlayerCount(){return sdatInfoEntry.player.entryCount;}
    int getGroupCount() {return sdatInfoEntry.group.entryCount;}
    int getPlayer2Count() {return sdatInfoEntry.player2.entryCount;}
    int getStrmCount() {return sdatInfoEntry.strm.entryCount;}

};

#define SDAT Sdat::Instance()