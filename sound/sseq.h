#pragma once

#include <cstdint>

class Sseq {
private:

public:

    struct InfoEntry {
        uint16_t fileID = 0; // 0x0 Um herauszufinden der wievielte Eintrag im Fat das ist
        // uint16_t unknown 0x2
        uint16_t bnk = 0; // 0x4 Verweist glaub ich auf die BNK(denke mal einfach z.B. bank nr1 oder nr3)
        uint8_t vol = 0; // 0x6 Volume
        uint8_t cpr = 0; // 0x7  Channel pressure
        uint8_t ppr = 0; // 0x8 Polyphonic pressure
        uint8_t ply = 0; // 0x9 Play
        // uint16_t unknown 0xA
    } infoEntry;

    struct Header {
        uint32_t id = 0;
        //uint16_t byteOrder = 0;
        //uint16_t version = 0;
        uint32_t fileSize = 0;
        //uint16_t headerSize = 0; // usually 0x10
        //uint16_t totalBlocks = 0; // (usually 1 = DATA)
        //uint32_t dataId = 0; // Subheader "DATA"
        //uint32_t fileSizeMinusTen = 0;
        uint32_t dataOffset = 0; // relative to sseq beginning
        // Sequenced data arrays ...
    } header;

    //https://www.feshrine.net/hacking/doc/nds-sdat.html#sseq
    //https://problemkaputt.de/gbatek-ds-sound-files-sseq-sound-sequence.htm
    
    /*struct Data {
        bool isMultiTrack = false; // Wenn dataOffset auf ein 'FE' Byte zeigt, ist es nen multiTrack. die 2 bytes danach sind die anzahl der tracks(immer 1 zu viel...)
        uint8_t trackCount = 0;


    } data;*/

    uint32_t dataOffset = 0; // Relative to SDAT File offset! NO ITS NOT ITS RELATIVE TO NDS ROM!!! STUPID *****
    uint32_t dataSize = 0;
    
    bool getHeader();
    //bool getData(sndType::Sseq& sseq);

};