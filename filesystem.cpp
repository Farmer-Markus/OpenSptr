#include <cstdint>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

#include "log.h"
#include "filesystem.h"
#include "byteutils.h"

// Am Anfang der ROM im header stehe die Werte, wo die
// eigentlichen Informationen für das Dateisystem und andere Daten stehen.
// Fat FileNameTable:
#define FNT_OFFSET 0x40
#define FNT_SIZE 0x44
// Fat Table:
#define FAT_OFFSET 0x48
#define FAT_SIZE 0x4C

// Rom path noch ändern!!
Filesystem::Filesystem() {
    if(!romStream.is_open())
        romStream.open("game.nds", std::ios::binary);

    if(!romStream.is_open()) {
        LOG.err("Failed to open rom file!");
        return;
    }
    
    // Read and save rom header Information
    romStream.seekg(FNT_OFFSET, std::ios::beg);
    romheader.fntOffset = BYTEUTILS.getLittleEndian(romStream, 4); // Read 4 Bytes

    romStream.seekg(FNT_SIZE, std::ios::beg);
    romheader.fntSize = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(FAT_OFFSET, std::ios::beg);
    romheader.fatOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(FAT_SIZE, std::ios::beg);
    romheader.fntSize = BYTEUTILS.getLittleEndian(romStream, 4);
}

Filesystem::~Filesystem() {
    if(romStream.is_open())
        romStream.close();

}

bool Filesystem::readFntTable(uint32_t dirOffset, std::filesystem::path path, File& file) {
    if(path.empty()) {
        file.offset = romheader.fntOffset;
        file.folder = true; // Root is a folder
        return true;
    }
    
    // Seek to Folder at beginning of FNT
    romStream.seekg(dirOffset, std::ios::beg);
    uint32_t contentOffset = BYTEUTILS.getLittleEndian(romStream, 4); // Offset von Ordnerinhalt im FNT Table (relativ zum FNT!)
    uint16_t contentId = BYTEUTILS.getLittleEndian(romStream, 2); // Id von erstem Inhalt vom Ordner (Im Fat damit Einträge auslesen)
    romStream.seekg(romheader.fntOffset + contentOffset, std::ios::beg); // Geht zum Anfang des nametables

    
    while(true) {
        uint8_t contentInfo = static_cast<uint8_t>(romStream.get()); // Get Info byte. If file byte=namelenght, if byte>0x80 than folder and rest is namelenght
        bool isFolder = false;
        if(contentInfo == 0x00)
            return false; // Failure ende von nametable erreicht, datei nicht gefunden!
                
        if(contentInfo >= 0x80) { // Then folder
            isFolder = true;
            contentInfo -= 0x80; // Länge von Ordnernamen
        }

        std::vector<char> buffer(contentInfo);
        romStream.read(buffer.data(), contentInfo);
        std::string name(buffer.begin(), buffer.end());
        buffer.clear();

        if(path.begin()->string() == name) { // Wenn nächstes Teil im path richtig ist dann
            if(!path.has_parent_path()) { // Wenn auch noch letzter Teil im path ist dann Fat Tabelle nach offsets durchsuchen
                if(isFolder) {
                    uint16_t dirID = BYTEUTILS.getLittleEndian(romStream, 2); // 2 Dir Bytes am Ende von dir Eintrag lesen
                    dirID = dirID & 0xFFF;
                    file.offset = romheader.fntOffset + (dirID * 8);
                    file.folder = true;
                    return true;
                } else {
                    romStream.seekg(romheader.fatOffset + (contentId * 8), std::ios::beg); // Zu den Fat Einträgen springen (1 Eintrag = 8 Bytes)
                    uint32_t dataOffset = BYTEUTILS.getLittleEndian(romStream, 4);

                    file.name = name;
                    file.offset = dataOffset;
                    file.size = BYTEUTILS.getLittleEndian(romStream, 4) - dataOffset;
                    return true;
                }
            } else { // Must be folder
                std::filesystem::path newPath;
                bool count = false;

                for(const auto& e : path) { // Delete first path piece
                    if(count)
                        newPath /= e;
                    count = true;
                }
            
                uint16_t dirID = BYTEUTILS.getLittleEndian(romStream, 2); // 2 Dir Bytes am Ende von dir Eintrag lesen
                dirID = dirID & 0xFFF; // Letzten Bits sind Offset für dir Liste(anfang von fnt)
                return readFntTable(romheader.fntOffset + (dirID * 8), newPath, file);
            }
        }

        if(isFolder) {
            romStream.seekg(2, std::ios::cur); // Folder Bytes überspringen
            continue;
        }

        contentId++;
    }
    
    return false; // Sollte nie eintreten
}

Filesystem::File Filesystem::getFile(std::filesystem::path path) {
    File file;
    if(!romStream.is_open())
        return file;

    
    if(!readFntTable(romheader.fntOffset, path, file))
        LOG.err("Failed to get File");

    return file;
}

std::vector<Filesystem::File> Filesystem::getDirContent(uint32_t offset) {
    std::vector<File> content;
    romStream.seekg(offset, std::ios::beg);
    uint32_t contentOffset = BYTEUTILS.getLittleEndian(romStream, 4);
    uint16_t contentId = BYTEUTILS.getLittleEndian(romStream, 2);
    romStream.seekg(romheader.fntOffset + contentOffset, std::ios::beg);

    while (true) {
        File file;
        
        uint8_t contentInfo = static_cast<uint8_t>(romStream.get()); // Get Info byte. If file byte=namelenght, if byte>0x80 than folder and rest is namelenght
        if(contentInfo == 0x00)
            return content; // Failure ende von nametable erreicht
                
        if(contentInfo >= 0x80) { // Then folder
            file.folder = true;
            contentInfo -= 0x80; // Länge von Ordnernamen
        }

        std::vector<char> buffer(contentInfo);
        romStream.read(buffer.data(), contentInfo);
        std::string name(buffer.begin(), buffer.end());
        buffer.clear();

        file.name = name;
        content.push_back(file);
        if(file.folder)
            romStream.ignore(2);
    }

    return content;
}