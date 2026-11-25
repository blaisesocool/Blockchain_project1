cmake_minimum_required(VERSION 3.16)
project(Blockchain_project1 LANGUAGES CXX)

add_executable(feistel feistel/feistel.cpp)
target_compile_features(feistel PRIVATE cxx_std_11)

add_executable(rsa RSA/rsa.cpp)
target_compile_features(rsa PRIVATE cxx_std_11)

# RSA needs the GMP big-integer library (brew install gmp)
find_path(GMP_INCLUDE_DIR gmpxx.h HINTS /opt/homebrew/include /usr/local/include)
find_library(GMP_LIB gmp HINTS /opt/homebrew/lib /usr/local/lib)
find_library(GMPXX_LIB gmpxx HINTS /opt/homebrew/lib /usr/local/lib)
target_include_directories(rsa PRIVATE ${GMP_INCLUDE_DIR})
target_link_libraries(rsa PRIVATE ${GMPXX_LIB} ${GMP_LIB})

# Homebrew in /usr/local is the Intel one, so its GMP is x86_64 only.
# Build rsa for x86_64 too (runs through Rosetta on Apple Silicon).
if(APPLE AND GMP_LIB MATCHES "^/usr/local/")
    set_target_properties(rsa PROPERTIES OSX_ARCHITECTURES x86_64)
endif()
