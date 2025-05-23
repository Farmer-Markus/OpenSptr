#pragma once

#include <cstdint>
#include <fstream>
#include <vector>
#include <filesystem>

class Byteutils {
private:

public:
    static Byteutils& Instance() {
        static Byteutils instance;
        return instance;
    }

    uint32_t getLittleEndian(std::ifstream &stream, uint8_t length);
    uint32_t getBytes(std::ifstream &stream, uint8_t lengtht);

    bool writeFile(std::vector<uint8_t>& data, std::filesystem::path path);
};

#define BYTEUTILS Byteutils::Instance()
// Erster Aufruf von BYTEUTILS AKA Byteutils::Instance() erstellt die
// instance und dann wird nurnoch die selbe instance übergeben.