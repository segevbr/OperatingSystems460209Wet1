cmake_minimum_required(VERSION 3.10)
project(smash)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(smash
        smash.cpp
        commands.cpp
        signals.cpp
        jobs.cpp
        fd_handler.cpp
)
