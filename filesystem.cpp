#include <cstdint>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>

#include "log.h"
#include "filesystem.h"
#include "byteutils.h"


#define TITLE_OFFSET 0x0 // 12 Bytes long
#define GAME_REGION 0x1D
// Am Anfang der ROM im header stehe die Werte, wo die
// eigentlichen Informationen für das Dateisystem und andere Daten stehen.
// Fat FileNameTable:
#define FNT_OFFSET 0x40
#define FNT_SIZE 0x44
// Fat Table:
#define FAT_OFFSET 0x48
#define FAT_SIZE 0x4C


Filesystem::~Filesystem() {
    if(romStream.is_open())
        romStream.close();

}

bool Filesystem::init(std::filesystem::path path) {
    romPath = path;
    
    if(!romStream.is_open()) {
        if(!newRomStream(romStream)) {
            LOG.err("Filesystem::init: Failed!");
            return false;
        }
    }
    
    // Read and save rom header Information
    romStream.seekg(TITLE_OFFSET, std::ios::beg);
    romStream.read((char*)romheader.title, 12);

    romStream.seekg(GAME_REGION, std::ios::beg);
    romheader.region = romStream.get();

    romStream.seekg(FNT_OFFSET, std::ios::beg);
    romheader.fntOffset = BYTEUTILS.getLittleEndian(romStream, 4); // Read 4 Bytes

    romStream.seekg(FNT_SIZE, std::ios::beg);
    romheader.fntSize = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(FAT_OFFSET, std::ios::beg);
    romheader.fatOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    romStream.seekg(FAT_SIZE, std::ios::beg);
    romheader.fntSize = BYTEUTILS.getLittleEndian(romStream, 4);
    
    LOG.debug("Filesystem::init: Rom opened successfully.");
    return true;
}

bool Filesystem::verifyRom(std::filesystem::path path) {
    std::ifstream fileStream(path, std::ios::binary);
    if(!fileStream.is_open()) {
        LOG.debug("Filesystem::verifyRom: Failed to open file '" + path.string() + "'");
        return false;
    }

    // Last byte needs to be '0' to compare with strcmp!
    char title[13] = {0};
    fileStream.seekg(TITLE_OFFSET, std::ios::beg);
    fileStream.read((char*)title, 12);
    if(std::strcmp(title, "SPIRITTRACKS")) { // strcmp returns true when chars not equal
        std::string titleStr(title);
        LOG.debug("Filesystem::verifyRom: Rom not valid! Game title is: '" + titleStr + "'.");
        fileStream.close();
        return false;
    }

    LOG.info("Filesystem::verifyRom: Found valid rom.");
    fileStream.close();
    return true;
}

bool Filesystem::searchRom(std::filesystem::path& foundPath, std::filesystem::path searchPath) {
    LOG.debug("Filesystem::searchRom: Searching for rom in '" + searchPath.string() + "'");
    for(const auto& entry : std::filesystem::directory_iterator(searchPath)) {
        if(entry.is_directory())
            continue;
        
        if(entry.file_size() < 100000000) // If file smaller than 100 MB(Rom is ca. 130MB)
            continue;

        if(verifyRom(entry.path())) {
            foundPath = entry.path();
            LOG.info("Found rom in '" + foundPath.string() + "'");
            return true;
        }
    }

    LOG.debug("Filesystem::searchRom: Was not able to find rom!");
    return false;
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

bool Filesystem::getFile(File& file, std::filesystem::path path) {
    LOG.debug("Filesystem::getFile: Trying to get File: '" + path.string() + "'");
    
    if(!romStream.is_open()) {
        LOG.err("Filesystem::getFile: RomStream not open!");
        return false;
    }

    
    if(!readFntTable(romheader.fntOffset, path, file)) {
        LOG.err("Filesystem::getFile: Failed to get File");
        return false;
    }

    return true;
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

bool Filesystem::extractRom(std::filesystem::path currPath, std::filesystem::path destPath) {
    // Get folder offset in folder table
    File currDir;
    getFile(currDir, currPath);

    // Use folder table offset to get content
    std::vector<File> dirContent = getDirContent(currDir.offset);

    for(File file : dirContent) {
        if(file.folder) {
            if(!std::filesystem::create_directories(destPath / currPath / file.name)) {
                LOG.err("Filesystem::extractRom: Failed to create directory: '" + (destPath / currPath / file.name).string() + "'");
                return false;
            }

            if(!extractRom(currPath / file.name, destPath))
                return false;
            continue;
        }

        if(!getFile(file, currPath/file.name))
            return false;

        
        romStream.seekg(file.offset, std::ios::beg);
        std::vector<uint8_t> dataBuffer(file.size);
        romStream.read((char*)dataBuffer.data(), file.size);

        if(!std::filesystem::is_directory(destPath / currPath)) {
            try {
                std::filesystem::create_directories(destPath / currPath);
            } catch (std::filesystem::filesystem_error& e) {
                LOG.fullErr("Filesystem::extractRom: Failed to create destination folder:", e.what());
                return false;
            }
        }

        if(!BYTEUTILS.writeFile(dataBuffer, destPath / currPath / file.name)) {
            LOG.err("Filesystem::extractRom: Failed to extract content!");
            return false;
        }
    }

    return true;
}