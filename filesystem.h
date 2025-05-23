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
        uint32_t offset = 0;
        uint32_t size = 0;
    };

    Filesystem(); // Beim Initialisieren
    ~Filesystem();
    
    std::ifstream& getRomStream() {
        return romStream;
    }

    File getFile(std::filesystem::path path);
};

#define FILESYSTEM Filesystem::Instance()