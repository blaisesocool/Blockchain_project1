cmake_minimum_required(VERSION 3.16)
project(Blockchain_project1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(feistel
    feistel/sbox.cpp
    feistel/keys.cpp
    feistel/feistel.cpp
    feistel/main.cpp
)

target_include_directories(feistel PRIVATE feistel)
target_compile_options(feistel PRIVATE -Wall -Wextra)