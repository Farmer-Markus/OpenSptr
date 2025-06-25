#include <cstdint>
#include <fstream>

#include "sseq.h"
#include "types.h"
#include "../filesystem.h"
#include "../byteutils.h"

#define ID 0x0
#define FILESIZE 0x8
//#define HEADERSIZE 0xC
#define DATA_OFFSET 0x18


bool Sseq::getHeader(sndType::Sseq& sseq) {
    std::ifstream& romStream = FILESYSTEM.getRomStream();
    sndType::Sseq::Header& header = sseq.header;
    
    romStream.seekg(sseq.dataOffset + ID, std::ios::beg);
    header.id = BYTEUTILS.getBytes(romStream, 4);

    if(header.id != 0x53534551) {
        LOG.hex("Failed to load SSEQ, wrong header ID:", header.id);
        return false;
    }

    romStream.seekg(sseq.dataOffset + FILESIZE, std::ios::beg);
    header.fileSize = BYTEUTILS.getLittleEndian(romStream, 4);

    //romStream.seekg(sseq.dataOffset + HEADERSIZE, std::ios::beg);
    //header.fileSize = BYTEUTILS.getLittleEndian(romStream, 2);

    romStream.seekg(sseq.dataOffset + DATA_OFFSET, std::ios::beg);
    header.dataOffset = BYTEUTILS.getLittleEndian(romStream, 4);

    return true;
}

