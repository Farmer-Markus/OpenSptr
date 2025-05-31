#pragma once

class Settings {
private:

public:
    static Settings& Instance() {
        static Settings instance;
        return instance;
    }

    bool cacheSounds = true; // Increases ram usage but reduces disc usage(Also able to reduce crackling when looping)
    bool showDebugOutput = true;
};

#define SETTINGS Settings::Instance()