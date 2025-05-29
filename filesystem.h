#pragma once

#include <cstdint>
#include <fstream>
#include <filesystem>

class Filesystem {
public:
    struct File;
    
private:
    std::ifstream romStream;
    
    // Information about Header of the Rom File
    struct Romheader {
        uint32_t fntOffset;
        uint32_t fntSize;
        uint32_t fatOffset;
        uint32_t fatSize;
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


    Filesystem(); // Beim Initialisieren
    ~Filesystem();
    
    std::ifstream& getRomStream() {
        return romStream;
    }

    File getFile(std::filesystem::path path);

    // @param offset: Offset(rom start) of dir in fnt(dir table)
    std::vector<File> getDirContent(uint32_t offset);
};

#define FILESYSTEM Filesystem::Instance()