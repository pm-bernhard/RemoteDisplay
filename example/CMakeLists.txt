project(RemoteDisplayExample)
cmake_minimum_required(VERSION 2.8.11)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_definitions("-std=gnu++0x")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core)
find_package(Qt5Gui)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})

set(SRC_LIST main.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QT_EXECUTEABLE_FLAGS}")

target_link_libraries(${PROJECT_NAME} RemoteDisplay
	${Qt5Gui_LIBRARIES}
	${Qt5Core_LIBRARIES})

include_directories(${RemoteDisplay_SOURCE_DIR})

if (WIN32 AND BUILD_SHARED_LIBS)
  #copy the needed shared library to the build dir
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_BINARY_DIR}/../src/RemoteDisplay.dll"
      ${CMAKE_CURRENT_BINARY_DIR}
     )
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
