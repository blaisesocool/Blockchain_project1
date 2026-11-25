cmake_minimum_required(VERSION 3.16)
project(Blockchain_project1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_library(twofish
    feistel/sbox.cpp
    feistel/keys.cpp
    feistel/feistel.cpp
    feistel/text.cpp
)

target_include_directories(twofish PUBLIC feistel)

add_executable(feistel feistel/main.cpp)
target_link_libraries(feistel PRIVATE twofish)

include(CTest)
if(BUILD_TESTING)
    add_executable(feistel_tests tests/feistel_tests.cpp)
    target_link_libraries(feistel_tests PRIVATE twofish)
    add_test(NAME feistel_correctness COMMAND feistel_tests)
endif()
