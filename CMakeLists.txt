include(FetchContent)
cmake_minimum_required(VERSION 3.26)
project(ladder)
set(CMAKE_CXX_STANDARD 17)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

# Boost
# Find Boost headers and some static (release only) libraries
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(OPENSSL_ROOT_DIR /usr/local/Cellar/openssl/1.0.2k)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

if(Boost_FOUND)
    message(STATUS found boost)
    include_directories(${Boost_INCLUDE_DIRS})
    add_executable(beast_demo main.cpp)
    target_link_libraries(beast_demo ${Boost_LIBRARIES})

endif()

add_executable(ladder main.cpp
        include/Player.h
        src/Player.cpp
        include/Match.h
        src/Match.cpp
        include/Ladder.h
        src/Ladder.cpp
        include/slacking.hpp
        include/json.hpp
        include/socket.hpp
)

target_link_libraries(ladder SQLite::SQLite3 curl OpenSSL::SSL OpenSSL::Crypto)

