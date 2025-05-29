#pragma once

#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include "../filesystem.h"
#include "../log.h"
#include "../byteutils.h"


#ifdef __unix__
    #define COLOR_RESET "\033[0m"
    #define COLOR_BLUE "\033[34m"
    #define COLOR_GREEN "\033[32m"
    #define COLOR_RED "\033[31m"
#else
    #define COLOR_RESET
    #define COLOR_BLUE
    #define COLOR_GREEN
    #define COLOR_RED
#endif


class Command {
private:

public:
    
    void help(std::string command) {
        if(command.empty()) {
            std::cout << "Tis is a interactive shell like enviroment." << std::endl;
            std::cout << "You are able to use commands to navigate inside the Rom Filesystem." << std::endl;
            std::cout << "Commands:" << std::endl;
            std::cout << "  exit    : To exit this enviroment." << std::endl;
            std::cout << "  ls      : Lists all Files and irectories in current directory." << std::endl;
            std::cout << "  cd      : Let you change your current directory." << std::endl;
            std::cout << "  clean   : Cleans the console output." << std::endl;
            std::cout << "  help    : Shows this menu." << std::endl;
            std::cout << "  extract : Extract file from rom to your disc." << std::endl;
            std::cout << "" << std::endl;
            std::cout << "To get further information about the commands type:" << std::endl;
            std::cout << "  help <command>" << std::endl;
            std::cout << "For example:" << std::endl;
            std::cout << "  help ls" << std::endl;
            return;
        }

        if(command == "ls") {
            std::cout << "Lists all Files and irectories in current or given directory." << std::endl;
            std::cout << "Example usage:" << std::endl;
            std::cout << "  ls           : Lists all Files and directories in current directory " << std::endl;
            std::cout << "  ls myFolder  : Lists all Files and directories in currentDir/myFolder" << std::endl;
            return;
        }

        if(command == "cd") {
            std::cout << "Change into given directory." << std::endl;
            std::cout << "Example usage:" << std::endl;
            std::cout << "  cd myfolder                 : Changes into 'myfolder'" << std::endl;
            std::cout << "  cd ..                       : Changes into parent directory" << std::endl;
            return;
        }

        if(command == "clean") {
            std::cout << "Cleans the console output." << std::endl;
            return;
        }

        if(command == "extract") {
            std::cout << "Extract file from rom to your disc." << std::endl;
            std::cout << "  extract <sourceFile> <Destination>" << std::endl;
            std::cout << "Example usage:" << std::endl;
            std::cout << "  extract sound.sdat /home/markus/Downloads/mySdat.sdat" << std::endl;
            std::cout << "" << std::endl;
            std::cout << "Destination path can also be '.' to extract to current directory." << std::endl;
        }
    }

#ifdef __unix__
    void clear() {
        std::cout << "\033[2J\033[1;1H";
    }
#else
    void clear() {
        std::cout << "Clear only supported on Unix like systems!" << std::endl;
    }
 #endif

    // Lists content of folder
    void list(std::filesystem::path oldPath) {
        std::filesystem::path path;
        
        if(!oldPath.empty() && oldPath.string().back() == '/')
            oldPath = oldPath.parent_path();

        for(const auto& e : oldPath) {
            if(e.string() == "..") {
                if(path.string().empty())
                    continue;
                path = path.parent_path(); // Letzten teil löschen, da ja ".." parent ordner heißt
            } else {
                path /= e;
            }

        }
        
        Filesystem::File file = FILESYSTEM.getFile(path);
        if(file.offset == 0x0)
            return;
        if(!file.folder) {
            std::cout << "ERROR: '"<< path.filename().string() << "' Is a File!" << std::endl;
            return; 
        }

        std::vector<std::string> folders;
        std::vector<std::string> files;

        std::vector<Filesystem::File> content = FILESYSTEM.getDirContent(file.offset);
        
        for(Filesystem::File& file : content) {
            if(file.folder) {
                folders.push_back(file.name);
            } else {
                files.push_back(file.name);
            }
        }

        std::cout << COLOR_BLUE;
        for(std::string folder : folders) {
            std::cout << folder << "  ";
        }

        std::cout << COLOR_RESET;
        for(std::string file : files) {
            std::cout << file << "  ";
        }

        std::cout << std::endl;
    }

    void changeDirectory(std::filesystem::path& currPath, std::filesystem::path newPath) {
        std::filesystem::path path = currPath;
        if(!newPath.empty() && newPath.string().back() == '/')
            newPath = newPath.parent_path();
        
        if(newPath.string() == "") {
            currPath = "";
            return;
        }

        for(const auto& e : newPath) {
            if(e.string() == "..") {
                if(path.string().empty())
                    continue;

                path = path.parent_path(); // Letzten teil löschen, da ja .. parent ordner heißt
            } else {
                path /= e;
            }

        }

        Filesystem::File file = FILESYSTEM.getFile(path);
        if(!file.folder) {
            std::cout << "Folder does not exist!" << std::endl;
        } else {
            currPath = path;
        }
    }

    void extract(std::filesystem::path path, std::filesystem::path destPath) {
        if(path.empty()) {
            std::cout << "Error: Source Path is missing!" << std::endl;
            return;
        }
        if(destPath.empty()) {
            std::cout << "Error: Destination Path is missing!" << std::endl;
            return;
        }

        Filesystem::File file = FILESYSTEM.getFile(path);
        std::ifstream& romStream = FILESYSTEM.getRomStream();
        romStream.seekg(file.offset, std::ios::beg);
        std::vector<uint8_t> buffer(file.size);
        romStream.read((char*)buffer.data(), file.size);

        if(std::filesystem::is_directory(destPath)) {
            destPath /= file.name;
        }

        if(std::filesystem::is_regular_file(destPath)) {
            std::cout << "Destination file does already exist!" << std::endl;
            return;
        }
        BYTEUTILS.writeFile(buffer, destPath);

        if(std::filesystem::is_regular_file(destPath)) {
            std::cout << COLOR_GREEN << "Extracted successfully!" << COLOR_RESET << std::endl;
            return;
        }
        std::cout << "Output file could not be created!" << std::endl;
    }
};