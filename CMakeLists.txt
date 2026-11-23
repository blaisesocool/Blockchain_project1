cmake_minimum_required(VERSION 3.16)
project(Blockchain_project1 LANGUAGES CXX)

add_executable(feistel feistel/feistel.cpp)
target_compile_features(feistel PRIVATE cxx_std_11)
