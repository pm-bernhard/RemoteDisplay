project(RemoteDisplay)
cmake_minimum_required(VERSION 2.8.11)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_definitions("-std=gnu++0x")
endif()
if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set (Qt5_NO_LINK_QTMAIN TRUE)    

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Multimedia)
find_package(Qt5Widgets)
find_package(FreeRDP)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Multimedia_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

if(QT_QTMULTIMEDIA_FOUND)
    set(WITH_QTSOUND 1)
endif(QT_QTMULTIMEDIA_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(${FreeRDP_INCLUDE_DIR} ${WinPR_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

#aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST
    global.h
    remotedisplaywidget_p.h
    freerdphelpers.h
    screenbuffer.h
    bitmaprectanglesink.h
    pointerchangesink.h
		cursorchangenotifier.cpp
		freerdpeventloop.cpp
		letterboxedscreenbuffer.cpp
		remotedisplaywidget.cpp
		scaledscreenbuffer.cpp
		freerdpclient.cpp
		freerdphelpers.cpp
		rdpqtsoundplugin.cpp
		remotescreenbuffer.cpp
)

# Default to build shared libs
if(NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
endif()

if (BUILD_SHARED_LIBS)
  MESSAGE("RemoteDisplay is build as SHARED library")
  add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
  # This property is needed in global.h to define correct DLL-Export Macros
  set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS REMOTEDISPLAY_LIBRARY)
else()
  MESSAGE("RemoteDisplay is build as STATIC library")
  add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
endif()

set (ADDITIONAL_LIBS 
		freerdp
		freerdp-client
		Qt5::Core
    Qt5::Widgets
    Qt5::Multimedia
    Qt5::Gui
    )

if (MSVC)
  set (ADDITIONAL_LIBS ${ADDITIONAL_LIBS} Qt5::WinMain)
endif()

target_link_libraries(${PROJECT_NAME} ${ADDITIONAL_LIBS})

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(FILES remotedisplaywidget.h global.h DESTINATION include/RemoteDisplay)
