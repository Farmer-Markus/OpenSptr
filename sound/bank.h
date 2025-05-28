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

    bool getHeader(sdatType::BNK& bnk);
    bool parse(sdatType::BNK& bnk);
};

#define BANK Bank::Instance()