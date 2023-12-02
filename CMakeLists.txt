cmake_minimum_required(VERSION 3.26)
project(ladder)

set(CMAKE_CXX_STANDARD 17)
find_package(SQLite3 REQUIRED)

add_executable(ladder main.cpp
        include/Player.h
        src/Player.cpp
        include/Match.h
        src/Match.cpp
        include/Ladder.h
        src/Ladder.cpp)

target_link_libraries(ladder SQLite::SQLite3)

