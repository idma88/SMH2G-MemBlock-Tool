cmake_minimum_required(VERSION 3.0)

project(MemBlockTool VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(src
    Main.cpp
    ErrorMessage.h
    ErrorMessage.cpp
    Logic.h
    Logic.cpp
    ../arduino/Protocol.h
)

file(GLOB ext ext/*.h ext/*.cpp)

file(GLOB ui ui/*.*)

### Groups ####
source_group(ui FILES ${ui})

### Includes & links ####
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${src} ${ext} ${ui})

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE "TRUE"
    )

    target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets)

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:NOREF /SAFESEH:NO /ENTRY:\"mainCRTStartup\"" )
endif(WIN32)