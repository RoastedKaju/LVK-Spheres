cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project("Spheres")

# Shader files
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/resources/shaders/*")

# Header files
file(GLOB_RECURSE HEADER_FILES "${CMAKE_SOURCE_DIR}/include/*.h")

add_executable(${PROJECT_NAME}
    "src/main.cpp"
    ${HEADER_FILES}
    ${SHADER_FILES})

# Includes
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Group the shader files in a folder
source_group("Shaders" FILES ${SHADER_FILES})

# Compile definations
target_compile_definitions(${PROJECT_NAME} PRIVATE SHADER_DIR="${CMAKE_SOURCE_DIR}/resources/shaders")

add_subdirectory("external/lvk")

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE LVKLibrary)
target_link_libraries(${PROJECT_NAME} PRIVATE LVKstb)
target_link_libraries(${PROJECT_NAME} PRIVATE ktx)
