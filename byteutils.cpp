#include <cstdint>
#include <fstream>

#include "byteutils.h"
#include "log.h"

// Liest 2 oder 4 Byte lange Werte aus und gibt sie als little endian zurück. Denkt dran der cursor geht 2 oder 4 scritte weiter!
uint32_t Byteutils::getLittleEndian(std::ifstream &stream, uint8_t length) {
    uint8_t byte[length];
    stream.read(reinterpret_cast<char*>(byte), length);

    uint32_t result;

    // Byte shifting. uint32_t besitzt 4 Bytes! (4 * 8 bits = 32 | 1 Byte = 8 Bits)
    // Zahlen und hexadezimal ist immer in bits gespeichert! Und wir schieben die bits nach links
    // hex:     0x6C = 01101100 in binär bzw: 00000000 00000000 00000000 01101100 in uint32_t
    // hex:     0x5F = 01011111 in binär    : 00000000 00000000 00000000 01011111 in uint32_t
    // uint32_t = (0x6C | 0x5F << 8); // Was das macht ist, es schreibt die 3 hin(00000000 00000000 00000000 01101100),
    // und schiebt dann die 5 acht bits davor(00000000 00000000 01011111 01101100) = 0x5F6C
    
    if(length == 2) {
        result = (byte[0] | byte[1] << 8);
    } else {
        result = (byte[0] | byte[1] << 8 | byte[2] << 16 | byte[3] << 24);
    }

    return result;
}

// Supports up to 4 Bytes! Big endian(like in hex editor)
uint32_t Byteutils::getBytes(std::ifstream &stream, uint8_t length) {
    uint8_t byte[length];
    stream.read(reinterpret_cast<char*>(byte), length);

    uint32_t result;

    if(length == 2) {
        result = (byte[1] | byte[0] << 8);
    } else {
        result = (byte[3] | byte[2] << 8 | byte[1] << 16 | byte[0] << 24);
    }

    return result;
}

bool Byteutils::writeFile(std::vector<uint8_t>& data, std::filesystem::path path) {
    std::ofstream of(path, std::ios::binary);
    if(!of.is_open()) {
        LOG.err("Failed opening output File: " + path.string());
        return false;
    }
    LOG.debug("Byteutils::writeFile: Path is open for writing! " + path.string());
    of.write(reinterpret_cast<const char*>(data.data()), data.size());
    of.close();
    LOG.debug("Byteutils::writeFile: File written and closed! " + path.string());

    return true;
}