project(RemoteDisplay-toplevel C CXX)
cmake_minimum_required (VERSION 3.2)

set(CMAKE_COLOR_MAKEFILE ON)

# search the FreeRDP Library
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(NOT DEFINED FREERDP_ROOT_DIR)
  set (FREERDP_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../FreeRDP")
  set (FREERDP_INCLUDE_DIR "${FREERDP_ROOT_DIR}/include")
endif()

find_package(FreeRDP REQUIRED)
MESSAGE("FREERDP_INCLUDE_DIR: ${FREERDP_INCLUDE_DIR}")

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Add LibRemoteDisplay
add_subdirectory(src)

# Add the client
add_subdirectory(client)
