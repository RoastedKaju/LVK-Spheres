cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("Spheres")

add_executable(${PROJECT_NAME} "src/main.cpp")

add_subdirectory("external/lvk")

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE LVKLibrary)
target_link_libraries(${PROJECT_NAME} PRIVATE LVKstb)
target_link_libraries(${PROJECT_NAME} PRIVATE ktx)
