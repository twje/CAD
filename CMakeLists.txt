cmake_minimum_required(VERSION 3.14)

project(CADProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include external dependencies using FetchContent
include(FetchContent)

# Add SFML dependency
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG master
)
FetchContent_MakeAvailable(SFML)

# Gather source files
file(GLOB_RECURSE CADSources 
    "src/*.cpp"
)

# Gather header files
file(GLOB_RECURSE CADHeaders 
    "src/*.h"
)

# Create static library
add_library(CADLibrary STATIC 
    ${CADSources}
    ${CADHeaders} # Helps with IDE navigation
)

target_link_libraries(CADLibrary PUBLIC 
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
    sfml-network
)

# Specify include directories
target_include_directories(CADLibrary PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SFML_SOURCE_DIR}/include
)

# Create the executable for the project
add_executable(${PROJECT_NAME} 
    src/Main.cpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    CADLibrary
)