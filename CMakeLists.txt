cmake_minimum_required(VERSION 3.10)
project(HairOnBaldModel)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-D_USE_MATH_DEFINES)

# Include directories for headers
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

# Link directories for libraries
link_directories(
    ${CMAKE_SOURCE_DIR}/lib
)

# Source files
set(SOURCES
    src/main.cpp
    src/ImGuiFileDialog.cpp
    src/shader.h
    src/model.h
    src/ImGuiFileDialog.h
    src/dirent.h
    src/ImGuiFileDialogConfig.h
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32  # For OpenGL (provided by Windows)
    glfw3dll  # For GLFW dynamic linking
    assimp-vc143-mt  # For Assimp dynamic linking
    glad
    imgui
)

# Copy DLLs to output directory (for runtime)
file(GLOB DLL_FILES "${CMAKE_SOURCE_DIR}/bin/*.dll")
file(COPY ${DLL_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Release)
file(COPY ${DLL_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Debug)

# Copy shaders and models to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})