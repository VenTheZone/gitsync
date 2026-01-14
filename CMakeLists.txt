cmake_minimum_required(VERSION 3.10)
project(GithubLazySync C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.c
    src/file_info.c
    src/duplicate_detector.c
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${OPENSSL_LIBRARIES}
    m  # Math library
)

# Include headers
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Werror
)

# Installation
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Testing
enable_testing()
add_subdirectory(tests)