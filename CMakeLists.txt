cmake_minimum_required(VERSION 3.10)
project(OpenSptr)

set(SOURCES_SUBDIRS )
macro(AddDirectory dir)
    file(GLOB SUB_FILES ${dir}/*.cpp ${dir}/*.h ${dir}/*.hpp)
    set(SOURCES_SUBDIRS ${SOURCES_SUBDIRS} ${SUB_FILES})
endmacro()

AddDirectory(sound)
#AddDirectory(shell) Not needed for header files


find_package(SDL2 REQUIRED)

add_executable(main main.cpp filesystem.cpp byteutils.cpp ${SOURCES_SUBDIRS})

target_link_libraries(main SDL2)
