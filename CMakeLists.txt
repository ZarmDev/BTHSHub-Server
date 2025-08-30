cmake_minimum_required(VERSION 3.13)
project(http-server-starter-cpp)

message(STATUS "CMake running in: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMake binary dir: ${CMAKE_BINARY_DIR}")

# Find all .cpp files in src directory and subdirectories
file(GLOB_RECURSE SOURCE_FILES 
  "src/*.cpp"
  "libs/*.cpp"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)

# Build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Thread preferences
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Allow files to include the include directory
include_directories(include)

# Find required packages from vcpkg
find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)
find_package(hiredis CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(podofo CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
# Taken from minisign
pkg_check_modules(LIBSODIUM libsodium)

# Add precompiled headers
add_library(project_options INTERFACE)

find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

target_precompile_headers(project_options INTERFACE
  <vector>
  <string>
  <unordered_map>
  <unordered_set>
  <iostream>
)

# Main executable
add_executable(server ${SOURCE_FILES})
if(TARGET server)
  message(STATUS "add_executable: Target 'server' was created successfully.")
else()
  message(FATAL_ERROR "add_executable: Target 'server' was NOT created! Check SOURCE_FILES: ${SOURCE_FILES}")
endif()

# Disable pico json
target_compile_definitions(server PRIVATE JWT_DISABLE_PICOJSON=0)
target_include_directories(server PUBLIC ${SODIUM_INCLUDE_DIR})
target_compile_options(server PUBLIC ${LIBSODIUM_CFLAGS} ${LIBSODIUM_CFLAGS_OTHER})
target_include_directories(server PRIVATE ${JWT_CPP_INCLUDE_DIRS})
target_link_libraries(server PRIVATE
  project_options
  Threads::Threads
  ZLIB::ZLIB
  hiredis::hiredis
  redis++::redis++_static
  pthread
  jwt-cpp::jwt-cpp
  nlohmann_json::nlohmann_json
  podofo::podofo
  ${LIBSODIUM_LIBRARIES}
)

# Compiler warnings
target_compile_options(server PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)