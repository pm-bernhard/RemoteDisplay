project(RemoteDisplay)
cmake_minimum_required (VERSION 3.0.2)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_definitions("-std=gnu++0x")
endif()
if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set (Qt5_NO_LINK_QTMAIN TRUE)

# Find the specific Qt libraries (subdependcies are includes automaticaly)
find_package(Qt5Core REQUIRED) #??needed?? -> detects Qt5::WinMain on Windows
find_package(Qt5Widgets REQUIRED)

include_directories(${FREERDP_INCLUDE_DIR})
include_directories(${WINPR_INCLUDE_DIR})
include_directories(${FREERDP_ROOT_DIR}/winpr/include)
include_directories(${FREERDP_ROOT_DIR}/include)
# TODO: find a better way to include this:
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../FreeRDP/winpr/include)

if(QT_QTMULTIMEDIA_FOUND)
  set(WITH_QTSOUND 1)
endif(QT_QTMULTIMEDIA_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

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
  MESSAGE("LibRemoteDisplay is build as SHARED library")
  add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
  # This property is needed in global.h to define correct DLL-Export Macros
  set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS REMOTEDISPLAY_LIBRARY)
else()
  MESSAGE("LibRemoteDisplay is build as STATIC library")
  add_library(${PROJECT_NAME} STATIC ${SRC_LIST})
endif()

# Link Qt-Modules
qt5_use_modules(${PROJECT_NAME} Widgets Core)

target_link_libraries(${PROJECT_NAME} freerdp freerdp-client)

if (WIN32)
  install(TARGETS ${PROJECT_NAME} DESTINATION . COMPONENT libraries EXPORT RemoteDisplay-Targets)
else()
  install(TARGETS ${PROJECT_NAME}
      RUNTIME DESTINATION ${REMOTEDISPLAY_INSTALL_PREFIX}bin
      LIBRARY DESTINATION ${REMOTEDISPLAY_INSTALL_PREFIX}lib
      ARCHIVE DESTINATION ${REMOTEDISPLAY_INSTALL_PREFIX}lib
  )
endif()
if (NOT REMOTEDISPLAY_SKIP_INSTALL_HEADER)
  install(FILES remotedisplaywidget.h global.h DESTINATION include/RemoteDisplay)
endif()
