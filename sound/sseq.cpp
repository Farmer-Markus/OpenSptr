#include <cstdint>
#include <fstream>

#include "sseq.h"
#include "../filesystem.h"
#include "../byteutils.h"

#define ID 0x0
#define FILESIZE 0x8
//#define HEADERSIZE 0xC
#define DATA_OFFSET 0x18 // Immer 1C


bool Sseq::getHeader() {
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    
    romStream.seekg(dataOffset + ID, std::ios::beg);
    header.id = BYTEUTILS.getBytes(romStream, 4);

    //                S S E Q
    if(header.id != 0x53534551) {
        LOG.hex("Failed to load SSEQ, wrong header ID:", header.id);
        return false;
    }

    romStream.seekg(dataOffset + FILESIZE, std::ios::beg);
    header.fileSize = BYTEUTILS.getLittleEndian(romStream, 4);

    //romStream.seekg(sseq.dataOffset + HEADERSIZE, std::ios::beg);
    //header.fileSize = BYTEUTILS.getLittleEndian(romStream, 2);

    romStream.seekg(dataOffset + DATA_OFFSET, std::ios::beg);
    header.dataOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    return true;
}

/*bool Sseq::getData(sndType::Sseq& sseq) {
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    sndType::Sseq::Header* header = &sseq.header;
    sndType::Sseq::Data* data = &sseq.data;

    romStream.seekg(sseq.dataOffset + header->dataOffset, std::ios::beg);



    return true;
}
*/