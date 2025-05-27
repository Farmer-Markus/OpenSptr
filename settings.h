class Settings {
private:

public:
    static Settings& Instance() {
        static Settings instance;
        return instance;
    }

    bool cacheSounds = false; // Increases ram usage but reduces disc usage
};

#define SETTINGS Settings::Instance()