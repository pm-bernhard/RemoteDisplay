project(RemoteDisplay-client C CXX)
cmake_minimum_required (VERSION 3.0.2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_definitions("-std=gnu++0x")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED) #??needed?? -> detects Qt5::WinMain on Windows
find_package(Qt5Widgets REQUIRED)

include_directories(${FREERDP_INCLUDE_DIR})
include_directories(${WINPR_INCLUDE_DIR})
include_directories(${FREERDP_ROOT_DIR}/winpr/include)
include_directories(${FREERDP_ROOT_DIR}/include)
# TODO: find a better way to include this:
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../FreeRDP/winpr/include)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRC_LIST main.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QT_EXECUTEABLE_FLAGS}")

# Link Qt-Modules
qt5_use_modules(${PROJECT_NAME} Widgets Core)

target_link_libraries(${PROJECT_NAME} RemoteDisplay)

include_directories(${RemoteDisplay_SOURCE_DIR})

#if (WIN32 AND BUILD_SHARED_LIBS)
#  #copy the needed shared library to the build dir
#  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#      COMMAND ${CMAKE_COMMAND} -E copy_if_different
#      "${CMAKE_CURRENT_BINARY_DIR}/../src/RemoteDisplay.dll"
#      ${CMAKE_CURRENT_BINARY_DIR}
#     )
#endif()

if (NOT REMOTEDISPLAY_SKIP_INSTALL_CLIENT)
  install(TARGETS ${PROJECT_NAME}
      RUNTIME DESTINATION ${REMOTEDISPLAY_INSTALL_PREFIX}bin
  )
endif()
