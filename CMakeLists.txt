cmake_minimum_required(VERSION 3.26)
project(ladder)

set(CMAKE_CXX_STANDARD 17)

add_executable(ladder main.cpp
        include/Player.h
        src/Player.cpp
        include/Match.h
        src/Match.cpp
        include/Ladder.h
        src/Ladder.cpp)
