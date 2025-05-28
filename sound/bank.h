#pragma once

#include "types.h"

#include <cstdint>
#include <vector>


class Bank {
private:

public:
    static Bank& Instance() {
        static Bank instance;
        return instance;
    }

    bool getHeader(sndType::Bank& bnk);
    bool parse(sndType::Bank& bnk);
};

#define BANK Bank::Instance()