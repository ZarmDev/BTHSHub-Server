cmake_minimum_required(VERSION 3.13)
project(http-server-starter-cpp)

# Remove unused SOURCE_FILES glob since we specify files explicitly

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)

# Build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Use ccache if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
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

add_library(database_objs OBJECT
  src/database/teamdatabase.cpp
  src/database/userdatabase.cpp
  src/database/hash.cpp
  src/database/jwt.cpp
  src/database/scheduledatabase.cpp 
)

add_library(pdf_objs OBJECT
  src/pdf/pdf.cpp
)

add_library(utils_objs OBJECT
  src/utils.cpp
  libs/lib.cpp
)

# Add routes as a separate object library
add_library(routes_objs OBJECT
  src/routes.cpp  # Add this missing file
)

# Add middleware as a separate object library  
add_library(middleware_objs OBJECT
  src/middleware.cpp  # You'll need to create this file
)

# Configure the object libraries to have access to dependencies
target_link_libraries(database_objs PRIVATE 
  project_options
  nlohmann_json::nlohmann_json
  jwt-cpp::jwt-cpp
  redis++::redis++_static
)

target_link_libraries(pdf_objs PRIVATE 
  project_options
  nlohmann_json::nlohmann_json
  podofo::podofo
)

target_link_libraries(utils_objs PRIVATE 
  project_options
)

target_link_libraries(routes_objs PRIVATE 
  project_options
  nlohmann_json::nlohmann_json
  jwt-cpp::jwt-cpp
)

target_link_libraries(middleware_objs PRIVATE 
  project_options
  jwt-cpp::jwt-cpp
)

find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/base.h")

target_precompile_headers(project_options INTERFACE
 <vector>
 <string>
 <unordered_map>
 <unordered_set>
 <iostream>
)

# Main executable
add_executable(server 
  src/server.cpp
  src/global.cpp
  $<TARGET_OBJECTS:database_objs>
  $<TARGET_OBJECTS:pdf_objs>
  $<TARGET_OBJECTS:utils_objs>
  $<TARGET_OBJECTS:routes_objs>
  $<TARGET_OBJECTS:middleware_objs>
)

target_compile_options(server PRIVATE
  -O0                    # No optimization
  -g0                    # No debug info at all
  -fno-rtti             # Disable runtime type info
  # -fno-exceptions       # Disable exceptions if not used
  -pipe                 # Use pipes, not temp files
)

# Only include stable external library headers in PCH
target_precompile_headers(server PRIVATE
  <sw/redis++/redis++.h>
  <nlohmann/json.hpp>
  <sodium.h>
  <sys/socket.h>     # Add Linux socket headers directly
  <netinet/in.h>     
  <arpa/inet.h>      
  <unistd.h>         
  <vector>
  <string>
  <unordered_map>
  <iostream>
  <thread>
  <mutex>
)

# Disable pico json
target_compile_definitions(server PRIVATE JWT_DISABLE_PICOJSON=1)
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