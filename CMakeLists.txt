cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("Spheres")

add_executable(${PROJECT_NAME} "src/main.cpp")