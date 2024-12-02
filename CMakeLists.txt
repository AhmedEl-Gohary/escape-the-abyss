cmake_minimum_required(VERSION 3.10)
project(EscapeTheAbyss)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files
file(GLOB SRC_FILES src/*.cpp)

# Find required libraries
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)

# For Assimp, manually specify include and library paths if CMake can't find it
find_path(ASSIMP_INCLUDE_DIR assimp/Importer.hpp PATHS /usr/include /usr/local/include)
find_library(ASSIMP_LIBRARY assimp PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)

if (NOT ASSIMP_INCLUDE_DIR OR NOT ASSIMP_LIBRARY)
    message(FATAL_ERROR "Could not find Assimp library or include path.")
endif()

include_directories(${ASSIMP_INCLUDE_DIR})

# Add executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        GLUT::GLUT
        GLEW::GLEW
        ${ASSIMP_LIBRARY}
)
