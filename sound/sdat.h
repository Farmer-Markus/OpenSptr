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
        infoList bnk;      // 0x10
        infoList swar;      // 0x14
        infoList player;    // 0x18
        infoList group;     // 0x1C
        infoList player2;   // 0x20
        infoList strm;      // 0x24
    } sdatInfoEntry;


    ~Sdat();

    bool loadSDAT(std::filesystem::path path);

    // Die countteste "" Datei zurück geben. Returns all 0 when file out of Range(provided number higher than actual files)
    void getSseq(sndType::Sseq& sseq, int count);
    void getSsar(sndType::Ssar& ssar, int count);
    void getBank(sndType::Bank& bnk, int count);
    void getSwar(sndType::Swar& swar, int count);
    void getPlayer(sndType::Player& player, int count);
    void getGroup(sndType::Group& group, int count);
    void getPlayer2(sndType::Player2& player2, int count);
    void getStrm(sndType::Strm& strm, int count);


    int getSseqCount() {return sdatInfoEntry.sseq.entryCount;}
    int getSsarCount() {return sdatInfoEntry.ssar.entryCount;}
    int getBankCount() {return sdatInfoEntry.bnk.entryCount;}
    int getSwarCount() {return sdatInfoEntry.swar.entryCount;}
    int getPlayerCount() {return sdatInfoEntry.player.entryCount;}
    int getGroupCount() {return sdatInfoEntry.group.entryCount;}
    int getPlayer2Count() {return sdatInfoEntry.player2.entryCount;}
    int getStrmCount() {return sdatInfoEntry.strm.entryCount;}

};

#define SDAT Sdat::Instance()