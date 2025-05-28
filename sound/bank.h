#pragma once

#include <cstdint>


class Bank {
private:

public:
    static Bank& Instance() {
        static Bank instance;
        return instance;
    }
};

#define BANK Bank::Instance()