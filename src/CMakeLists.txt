project(RemoteDisplay)

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    add_definitions("-std=gnu++0x")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Multimedia)
find_package(Qt5Widgets)
find_package(WinPR)
find_package(FreeRDP)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Multimedia_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

if(QT_QTMULTIMEDIA_FOUND)
    set(WITH_QTSOUND 1)
endif(QT_QTMULTIMEDIA_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include("${QT_USE_FILE}")
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

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS REMOTEDISPLAY_LIBRARY)
target_link_libraries(${PROJECT_NAME}
		freerdp
		freerdp-client
		${Qt5Gui_LIBRARIES}
		${Qt5Core_LIBRARIES}
		${Qt5Widgets_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(FILES remotedisplaywidget.h global.h DESTINATION include/RemoteDisplay)
