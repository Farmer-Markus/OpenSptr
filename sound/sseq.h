#pragma once

#include <cstdint>

#include "types.h"

class Sseq {
private:

public:
    static Sseq& Instance() {
            static Sseq instance;
            return instance;
        }
    
    bool getHeader(sndType::Sseq& sseq);
    //bool getData(sndType::Sseq& sseq);

};

#define SSEQ Sseq::Instance()