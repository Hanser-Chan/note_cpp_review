cmake_minimum_required(VERSION 3.20)

project(cpp20_lab LANGUAGES CXX)

add_executable(cpp20_lab
    src/main.cpp
)

target_compile_features(cpp20_lab PRIVATE cxx_std_20)
