cmake_minimum_required(VERSION 3.10)
project(zelda-spirit-cpp)

set(SOURCES_SUBDIRS )
macro(AddDirectory dir)
    file(GLOB SUB_FILES ${dir}/*.cpp ${dir}/*.h ${dir}/*.hpp ${dir}/*.tpp)
    set(SOURCES_SUBDIRS ${SOURCES_SUBDIRS} ${SUB_FILES})
endmacro()

AddDirectory(sound)


find_package(SDL2 REQUIRED)

add_executable(main main.cpp filesystem.cpp byteutils.cpp ${SOURCES_SUBDIRS})

target_link_libraries(main SDL2)
