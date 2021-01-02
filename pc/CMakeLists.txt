cmake_minimum_required(VERSION 2.6)

project(MemBlockTool)

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.59.0 REQUIRED system filesystem)
find_package(Qt4 REQUIRED QtCore QtGui)

set(QT_USE_QTMAIN TRUE)
include(${QT_USE_FILE})

set(src
    main.cpp
	logic.h
	logic.cpp
)

set(ext
    ext/xserial.hpp
    ext/xserial.cpp
)


set(ui_h
    ui/MainWindow.h
)

set(ui_cpp
    ui/MainWindow.cpp
)

set(ui
    ${ui_h}
    ${ui_cpp}
)

### QT ### 
set(MOC_HEADERS ${ui_h})
QT4_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})

set(UI_FORMS ui/MainWindow.ui)
qt4_wrap_ui(UI_HEADERS ${UI_FORMS})

set(UI_RESOURCES ui/ui.qrc)
QT4_add_resources(UI_RESOURCES_QRC ${UI_RESOURCES})

### Groups ####
source_group(ui			FILES ${ui_h} ${ui_cpp} ${UI_FORMS} ${UI_RESOURCES})
source_group(ui\\moc	FILES ${MOC_SOURCES} ${UI_HEADERS})

### Includes & links ####
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS}) 

add_executable(${PROJECT_NAME}
	${src} ${ext}
	${ui} ${UI_FORMS} ${UI_RESOURCES}
	${MOC_SOURCES} ${UI_HEADERS} ${UI_RESOURCES_QRC}
)

if(WIN32)
#  set(COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS};_SCL_SECURE_NO_WARNINGS;OPJ_STATIC")

    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE "TRUE"
    )

    target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${QT_LIBRARIES})

#    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "${COMPILE_DEFINITIONS}")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /OPT:NOREF /SAFESEH:NO /ENTRY:\"mainCRTStartup\"" )
endif(WIN32)