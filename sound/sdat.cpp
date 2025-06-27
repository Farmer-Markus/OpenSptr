#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>

#include "sdat.h"
#include "sseq.h"
#include "bnk.h"
#include "swar.h"
#include "strm.h"
#include "../log.h"
#include "../filesystem.h"
#include "../byteutils.h"

// SDAT File -> INFO BLOCK(offsets to lists of sseq, ssar ...) -> LIST (anzahl der Einträge,
// Danach  einträge * Offsets zu -> INFO_ENTRIES(für jeden typ ein einzelner eintrag, 
// Da stehen ID von datei und andere werte) -> ID * 16(jeder eintrag in fat = 16 bytes groß)
// FAT+12(header)+(ID*16) dann ersten 4(0-3) bytes -> Offset in FILE tabelle(von sdat start)
// nächsten 4 Bytes(4-8) größe von datei,letzten 8 sind ungenutzt/platzhalter


#define ID 0x0 // ID: 4 Bytes "SDAT"

#define INFO_OFFSET 0x18
#define INFO_SIZE 0x1C // Ja das stimmt so!
// SDAT Fat Table information(located at header of SDAT)
#define FAT_OFFSET 0x20
#define FAT_SIZE 0x24

#define FILE_OFFSET 0x28
#define FILE_SIZE 0x2C

#define INFO_SSEQ_OFFSET 0x8
#define INFO_SSAR_OFFSET 0xC
#define INFO_BANK_OFFSET 0x10
#define INFO_SWAR_OFFSET 0x14
#define INFO_PLAYER_OFFSET 0x18
#define INFO_GROUP_OFFSET 0x1C
#define INFO_PLAYER2_OFFSET 0x20
#define INFO_STRM_OFFSET 0x24


// Data of Header of Fat inside SDAT
#define FAT_ID 0x0 // relativ zum anfang der fat tabelle
#define FAT_FILES 0x8 // Number of Files in Fat table
#define FAT_ENTRIES 0xC // Start of fat entries( entry: 12 Bytes: 4+4+8 offset, size, idk wofür...)
#define FAT_ENTRY_SIZE 0x10 // 1 Fat Eintrag ist 16 Bytes lang


// Loads SDAT File(overwrites currently loaded sdat!)
bool Sdat::loadSDAT(std::filesystem::path path) {
    FILESYSTEM.getFile(sdat, path); // sdat File defined in sound.h

    if(sdat.offset == 0) {
        LOG.err("Failed to load SDAT! Sdat offset = 0");
        return false;
    }

    std::ifstream& romStream = FILESYSTEM.getRomStream();
    romStream.seekg(sdat.offset + ID, std::ios::beg);
    header.id = BYTEUTILS.getBytes(romStream, 4);

    if(header.id != 0x53444154) { // If header does not begin with "SDAT"(in hex)
        LOG.err("Failed to load SDAT! Header ID != SDAT.");
        LOG.hex("Header ID:", header.id);
        return false;
    }

    // Get Offset and Size of INFO Table
    romStream.seekg(sdat.offset + INFO_OFFSET, std::ios::beg);
    header.infoOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(sdat.offset + INFO_SIZE, std::ios::beg);
    header.infoSize = BYTEUTILS.getLittleEndian(romStream, 4);

    // Get offset and Size of FAT Table
    romStream.seekg(sdat.offset + FAT_OFFSET, std::ios::beg);
    header.fatOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(sdat.offset + FAT_SIZE, std::ios::beg);
    header.fatSize = BYTEUTILS.getLittleEndian(romStream, 4);

    // Get Offset and Size of FILE Table
    romStream.seekg(sdat.offset + FILE_OFFSET, std::ios::beg);
    header.fileOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(sdat.offset + FILE_SIZE, std::ios::beg);
    header.fileSize = BYTEUTILS.getLittleEndian(romStream, 4);


    uint32_t infoOffset = sdat.offset + header.infoOffset;
    // Parse Info Block and Info List entries | -> Info Block -> Offset to Info List -> Collect data about sound types
    romStream.seekg(infoOffset + INFO_SSEQ_OFFSET, std::ios::beg); // Go to Information Block in sdat to offset of sseq iformation
    uint32_t sseqOffset = BYTEUTILS.getLittleEndian(romStream, 4); // Get offset for sseq list
    romStream.seekg(infoOffset + sseqOffset, std::ios::beg);  // Go to offset of sseq list
    sdatInfoEntry.sseq.entryCount = BYTEUTILS.getLittleEndian(romStream, 4); // Read first 4 bytes (sseq file count)
    for(size_t count = 0; count < sdatInfoEntry.sseq.entryCount; count++) { // For entrycount times read 4 bytes(every 4 bytes are offsets to sseq file infos)
        uint32_t entry = BYTEUTILS.getLittleEndian(romStream, 4);
        sdatInfoEntry.sseq.entries.push_back(entry); // Gibt immer wieder welche die die selbe ID haben, da ist dann nur die geschwindigkeit anders oder der pitch!
    }

    romStream.seekg(infoOffset + INFO_SSAR_OFFSET, std::ios::beg); // Same for SSAR and every other entry
    uint32_t ssarOffset = BYTEUTILS.getLittleEndian(romStream, 4);
    romStream.seekg(infoOffset + ssarOffset, std::ios::beg);
    sdatInfoEntry.ssar.entryCount = BYTEUTILS.getLittleEndian(romStream, 4);
    for(size_t count = 0; count < sdatInfoEntry.ssar.entryCount; count++) {
        uint32_t entry = BYTEUTILS.getLittleEndian(romStream, 4);
        sdatInfoEntry.ssar.entries.push_back(entry);
    }

    romStream.seekg(infoOffset + INFO_BANK_OFFSET, std::ios::beg);
    uint32_t bankOffset = BYTEUTILS.getLittleEndian(romStream, 4);
    romStream.seekg(infoOffset + bankOffset, std::ios::beg);
    sdatInfoEntry.bnk.entryCount = BYTEUTILS.getLittleEndian(romStream, 4);
    for(size_t count = 0; count < sdatInfoEntry.bnk.entryCount; count++) {
        uint32_t entry = BYTEUTILS.getLittleEndian(romStream, 4);
        sdatInfoEntry.bnk.entries.push_back(entry);
    }

    romStream.seekg(infoOffset + INFO_SWAR_OFFSET, std::ios::beg);
    uint32_t swarOffset = BYTEUTILS.getLittleEndian(romStream, 4);
    romStream.seekg(infoOffset + swarOffset, std::ios::beg);
    sdatInfoEntry.swar.entryCount = BYTEUTILS.getLittleEndian(romStream, 4);
    for(size_t count = 0; count < sdatInfoEntry.swar.entryCount; count++) {
        uint32_t entry = BYTEUTILS.getLittleEndian(romStream, 4);
        sdatInfoEntry.swar.entries.push_back(entry);
    }

    romStream.seekg(infoOffset + INFO_PLAYER_OFFSET, std::ios::beg);
    uint32_t playerOffset = BYTEUTILS.getLittleEndian(romStream, 4);
    romStream.seekg(infoOffset + playerOffset, std::ios::beg);
    sdatInfoEntry.player.entryCount = BYTEUTILS.getLittleEndian(romStream, 4);
    for(size_t count = 0; count < sdatInfoEntry.player.entryCount; count++) {
        uint32_t entry = BYTEUTILS.getLittleEndian(romStream, 4);
        sdatInfoEntry.player.entries.push_back(entry);
    }

    romStream.seekg(infoOffset + INFO_GROUP_OFFSET, std::ios::beg);
    uint32_t groupOffset = BYTEUTILS.getLittleEndian(romStream, 4);
    romStream.seekg(infoOffset + groupOffset, std::ios::beg);
    sdatInfoEntry.group.entryCount = BYTEUTILS.getLittleEndian(romStream, 4);
    for(size_t count = 0; count < sdatInfoEntry.group.entryCount; count++) {
        uint32_t entry = BYTEUTILS.getLittleEndian(romStream, 4);
        sdatInfoEntry.group.entries.push_back(entry);
    }

    romStream.seekg(infoOffset + INFO_PLAYER2_OFFSET, std::ios::beg);
    uint32_t player2Offset = BYTEUTILS.getLittleEndian(romStream, 4);
    romStream.seekg(infoOffset + player2Offset, std::ios::beg);
    sdatInfoEntry.player2.entryCount = BYTEUTILS.getLittleEndian(romStream, 4);
    for(size_t count = 0; count < sdatInfoEntry.player2.entryCount; count++) {
        uint32_t entry = BYTEUTILS.getLittleEndian(romStream, 4);
        sdatInfoEntry.player2.entries.push_back(entry);
    }

    romStream.seekg(infoOffset + INFO_STRM_OFFSET, std::ios::beg);
    uint32_t strmOffset = BYTEUTILS.getLittleEndian(romStream, 4);
    romStream.seekg(infoOffset + strmOffset, std::ios::beg);
    sdatInfoEntry.strm.entryCount = BYTEUTILS.getLittleEndian(romStream, 4);
    for(size_t count = 0; count < sdatInfoEntry.strm.entryCount; count++) {
        uint32_t entry = BYTEUTILS.getLittleEndian(romStream, 4);
        sdatInfoEntry.strm.entries.push_back(entry);
    }

    LOG.debug("Sdat::loadSDAT: SDAT loaded successfully.");
    return true;
}



// Alles Funktionen um die Offsets und anderen Informationen in den gegebenen
// class pointer zu schreiben.
void Sdat::getSseq(Sseq& sseq, int count) {
    if(count >= sdatInfoEntry.sseq.entryCount)
        return;
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    if(sdatInfoEntry.sseq.entries[count] == 0x00000000 || sdatInfoEntry.sseq.entries[count] == 0xFFFFFFFF)
        return;

    romStream.seekg(sdat.offset + header.infoOffset + sdatInfoEntry.sseq.entries[count], std::ios::beg);

    // Info Eintrag füllen
    sseq.infoEntry.fileID = BYTEUTILS.getLittleEndian(romStream, 2);
    romStream.ignore(2); // uint16_t unknown 0x2
    sseq.infoEntry.bnk = BYTEUTILS.getLittleEndian(romStream, 2);
    sseq.infoEntry.vol = static_cast<uint8_t>(romStream.get());
    sseq.infoEntry.cpr = static_cast<uint8_t>(romStream.get());
    sseq.infoEntry.ppr = static_cast<uint8_t>(romStream.get());
    sseq.infoEntry.ply = static_cast<uint8_t>(romStream.get());

    uint32_t fatOffset = sdat.offset + header.fatOffset + FAT_ENTRIES + (sseq.infoEntry.fileID * FAT_ENTRY_SIZE);
    romStream.seekg(fatOffset, std::ios::beg);

    sseq.dataOffset = sdat.offset + BYTEUTILS.getLittleEndian(romStream, 4);
    sseq.dataSize = BYTEUTILS.getLittleEndian(romStream, 4);
}

/*void Sdat::getSsar(Ssar& ssar, int count) {
    if(count >= sdatInfoEntry.ssar.entryCount)
        return;
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    if(sdatInfoEntry.ssar.entries[count] == 0x00000000 || sdatInfoEntry.ssar.entries[count] == 0xFFFFFFFF)
        return;

    romStream.seekg(sdat.offset + header.infoOffset + sdatInfoEntry.ssar.entries[count], std::ios::beg);

    ssar.infoEntry.fileID = BYTEUTILS.getLittleEndian(romStream, 2);

    uint32_t fatOffset = sdat.offset + header.fatOffset + FAT_ENTRIES + (ssar.infoEntry.fileID * FAT_ENTRY_SIZE);
    romStream.seekg(fatOffset, std::ios::beg);

    ssar.dataOffset = sdat.offset + BYTEUTILS.getLittleEndian(romStream, 4);
    ssar.dataSize = BYTEUTILS.getLittleEndian(romStream, 4);
}*/

void Sdat::getBnk(Bnk& bnk, int count) {
    if(count >= sdatInfoEntry.bnk.entryCount)
        return;
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    if(sdatInfoEntry.bnk.entries[count] == 0x00000000 || sdatInfoEntry.bnk.entries[count] == 0xFFFFFFFF)
        return;

    romStream.seekg(sdat.offset + header.infoOffset + sdatInfoEntry.bnk.entries[count], std::ios::beg);

    bnk.infoEntry.fileID = BYTEUTILS.getLittleEndian(romStream, 2);
    romStream.ignore(2); // uint16_t unknown 0x2
    bnk.infoEntry.swar[0] = BYTEUTILS.getLittleEndian(romStream, 2);
    bnk.infoEntry.swar[1] = BYTEUTILS.getLittleEndian(romStream, 2);
    bnk.infoEntry.swar[2] = BYTEUTILS.getLittleEndian(romStream, 2);
    bnk.infoEntry.swar[3] = BYTEUTILS.getLittleEndian(romStream, 2);

    uint32_t fatOffset = sdat.offset + header.fatOffset + FAT_ENTRIES + (bnk.infoEntry.fileID * FAT_ENTRY_SIZE);
    romStream.seekg(fatOffset, std::ios::beg);

    bnk.dataOffset = sdat.offset + BYTEUTILS.getLittleEndian(romStream, 4);
    bnk.dataSize = BYTEUTILS.getLittleEndian(romStream, 4);
}

void Sdat::getSwar(Swar& swar, int count) {
    if(count >= sdatInfoEntry.swar.entryCount)
        return;
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    if(sdatInfoEntry.swar.entries[count] == 0x00000000 || sdatInfoEntry.swar.entries[count] == 0xFFFFFFFF)
        return;

    romStream.seekg(sdat.offset + header.infoOffset + sdatInfoEntry.swar.entries[count], std::ios::beg);

    swar.infoEntry.fileID = BYTEUTILS.getLittleEndian(romStream, 2);

    uint32_t fatOffset = sdat.offset + header.fatOffset + FAT_ENTRIES + (swar.infoEntry.fileID * FAT_ENTRY_SIZE);
    romStream.seekg(fatOffset, std::ios::beg);

    swar.dataOffset = sdat.offset + BYTEUTILS.getLittleEndian(romStream, 4);
    swar.dataSize = BYTEUTILS.getLittleEndian(romStream, 4);
}

/*void Sdat::getPlayer(Player& player, int count) {
    if(count >= sdatInfoEntry.player.entryCount)
        return;
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    if(sdatInfoEntry.player.entries[count] == 0x00000000 || sdatInfoEntry.player.entries[count] == 0xFFFFFFFF)
        return;

    romStream.ignore(1); // uint8_t unknown 0x0
    romStream.seekg(sdat.offset + header.infoOffset + sdatInfoEntry.player.entries[count], std::ios::beg);

    romStream.seekg(1, std::ios::cur); // 1 Byte überspringen, da nicht bekannt ist wofür das ist
    player.infoEntry.padding = BYTEUTILS.getLittleEndian(romStream, 2);
}*/

/*void Sdat::getGroup(Group& group, int count) {
    if(count >= sdatInfoEntry.group.entryCount)
        return;
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    if(sdatInfoEntry.group.entries[count] == 0x00000000 || sdatInfoEntry.group.entries[count] == 0xFFFFFFFF)
        return;

    romStream.seekg(sdat.offset + header.infoOffset + sdatInfoEntry.group.entries[count], std::ios::beg);

    group.infoEntry.itemCount = BYTEUTILS.getLittleEndian(romStream, 2);
    
    // NOCHMAL ÜBERPRÜFEN bin mir nicht sicher ob das so soll...
    for(size_t count = 0; count < group.infoEntry.itemCount; count++) {
        Group::InfoEntry::group grp;
        grp.type = BYTEUTILS.getLittleEndian(romStream, 4);
        grp.nEntry = BYTEUTILS.getLittleEndian(romStream, 4);
        group.infoEntry.groups.push_back(grp);
    }
}*/

/*void Sdat::getPlayer2(Player2& player2, int count) {
    if(count >= sdatInfoEntry.player2.entryCount)
        return;
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    if(sdatInfoEntry.player2.entries[count] == 0x00000000 || sdatInfoEntry.player2.entries[count] == 0xFFFFFFFF)
        return;

    romStream.seekg(sdat.offset + header.infoOffset + sdatInfoEntry.player2.entries[count], std::ios::beg);

    romStream.seekg(1, std::ios::cur); // 1 Byte überspringen, da nicht bekannt ist wofür das ist
    player2.infoEntry.nCount = BYTEUTILS.getLittleEndian(romStream, 2);
    // Unknown array aber maybe trotzdem einbauen!?
}*/

void Sdat::getStrm(Strm& strm, int count) {
    if(count >= sdatInfoEntry.strm.entryCount)
        return;
    
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    if(sdatInfoEntry.strm.entries[count] == 0x00000000 || sdatInfoEntry.strm.entries[count] == 0xFFFFFFFF)
        return;

    romStream.seekg(sdat.offset + header.infoOffset + sdatInfoEntry.strm.entries[count], std::ios::beg);

    strm.infoEntry.fileID = BYTEUTILS.getLittleEndian(romStream, 2);
    romStream.ignore(2); // uint16_t unknown 0x2
    strm.infoEntry.vol = static_cast<uint8_t>(romStream.get());
    strm.infoEntry.pri = static_cast<uint8_t>(romStream.get());
    strm.infoEntry.ply = static_cast<uint8_t>(romStream.get());

    uint32_t fatOffset = sdat.offset + header.fatOffset + FAT_ENTRIES + (strm.infoEntry.fileID * FAT_ENTRY_SIZE);
    romStream.seekg(fatOffset, std::ios::beg);

    strm.dataOffset = sdat.offset + BYTEUTILS.getLittleEndian(romStream, 4);
    strm.dataSize = BYTEUTILS.getLittleEndian(romStream, 4);
}