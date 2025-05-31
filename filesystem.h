#pragma once

#include <cstdint>
#include <fstream>
#include <filesystem>

#include "log.h"


class Filesystem {
public:
    struct File;
    
private:
    std::filesystem::path romPath;
    std::ifstream romStream;
    
    // Information about Header of the Rom File
    struct Romheader {
        char title[12] = {0};
        // ..
        uint8_t region = 0; // (00h=Normal, 80h=China, 40h=Korea)
        // ..
        uint32_t fntOffset = 0;
        uint32_t fntSize = 0;
        uint32_t fatOffset = 0;
        uint32_t fatSize = 0;
        // ..
    } romheader;

    bool readFntTable(uint32_t dirOffset, std::filesystem::path path, File& file);

public:
    static Filesystem& Instance() {
        static Filesystem instance;
        return instance;
    }
    
    // Just offset and size of file in rom not actual data
    struct File {
        bool folder = false;
        std::string name = "";
        uint32_t offset = 0;
        uint32_t size = 0;
    };

    ~Filesystem();

    // Loads the rom and reads the header
    // @param path : Path to game rom
    bool init(std::filesystem::path path);

    // Checks given file for SPIRITTRACKS header and return true when found
    bool verifyRom(std::filesystem::path path);

    // Search for rom in specific folder
    // @param foundPath : When found the rom path will be written in there
    // @param searchPath : Path to search for rom
    bool searchRom(std::filesystem::path& foundPath, std::filesystem::path searchPath);

    // Opens given ifstream to rom file
    bool newRomStream(std::ifstream& stream) {
        stream.open(romPath, std::ios::binary);

        if(stream.is_open())
            return true;
        
        LOG.err("Filesystem::newRomStream: Failed to open new rom stream!");
        return false;
    }
    
    std::ifstream& getRomStream() {return romStream;}

    bool getFile(File& file, std::filesystem::path path);

    // @param offset: Offset(rom start) of dir in fnt(dir table)
    std::vector<File> getDirContent(uint32_t offset);
};

#define FILESYSTEM Filesystem::Instance()