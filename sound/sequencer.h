#pragma once

#include <fstream>
#include <stdint.h>

#include "types.h"


class Sequencer {
private:
    sndType::Sseq sseq;
    

public:
    Sequencer(sndType::Sseq sseq) {
        this->sseq = sseq;
    }

    bool parseEvent(std::ifstream& in, uint32_t offset);
};