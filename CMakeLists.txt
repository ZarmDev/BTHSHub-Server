cmake_minimum_required(VERSION 3.13)
project(http-server-starter-cpp)

# Find all .cpp files in src directory and subdirectories
file(GLOB_RECURSE SOURCE_FILES 
  "src/*.cpp"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Thread preferences
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)

# Add precompiled headers
add_library(project_options INTERFACE)
target_precompile_headers(project_options INTERFACE
  <vector>
  <string>
  <unordered_map>
  <unordered_set>
  <iostream>
)

# Main executable
add_executable(server ${SOURCE_FILES})
target_link_libraries(server PRIVATE
  project_options
  Threads::Threads
  ZLIB::ZLIB
  hiredis
  redis++
  pthread
  jwt-cpp::jwt-cpp
)

# Compiler warnings
target_compile_options(server PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)