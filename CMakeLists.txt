cmake_minimum_required(VERSION 3.15)

project(pvztoolkit)

# set(CMAKE_BUILD_TYPE MinSizeRel)
# set(FLTK_DIR "C:/Dev/fltk_1.4.0/msvc2017_x86/static/CMake")

option(USE_QT_UI "Build with Qt modern UI instead of FLTK classic UI" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(USE_QT_UI)
    message(STATUS "Building with Qt Modern UI")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
    
    set(PZTK_SRCS
        ./src/code.cpp
        ./src/data.cpp
        ./src/checkcombobox.cpp
        ./src/lineup.cpp
        ./src/main_qt.cpp
        ./src/pak.cpp
        ./src/process.cpp
        ./src/pvz.cpp
        ./src/toolkit_qt.cpp
        ./src/utils.cpp
        ./src/window_qt.cpp
    )
    set(PZTK_QT_HEADERS
        ./inc/checkcombobox.h
        ./inc/toolkit_qt.h
        ./inc/window_qt.h
    )
    message(STATUS "Qt sources: ${PZTK_SRCS}")
else()
    message(STATUS "Building with FLTK Classic UI")
    find_package(FLTK CONFIG REQUIRED)
    
    set(PZTK_SRCS
        ./src/code.cpp
        ./src/data.cpp
        ./src/lineup.cpp
        ./src/main.cpp
        ./src/pak.cpp
        ./src/process.cpp
        ./src/pvz.cpp
        ./src/toolkit.cpp
        ./src/utils.cpp
        ./src/window.cpp
    )
    message(STATUS "FLTK sources: ${PZTK_SRCS}")
endif()

add_executable(pvztoolkit WIN32 ${PZTK_SRCS})

if(USE_QT_UI)
    target_sources(pvztoolkit PRIVATE ${PZTK_QT_HEADERS})
endif()

# target_include_directories(
#     pvztoolkit PRIVATE
#     ./inc
# )

target_sources(
    pvztoolkit PRIVATE
    ./res/version.rc
    ./res/pvztoolkit.manifest
)

target_compile_definitions(
    pvztoolkit PRIVATE
    UNICODE _UNICODE
    WIN32_LEAN_AND_MEAN NOMINMAX
    _WIN32_WINNT=0x0600
    _REGEX_MAX_STACK_COUNT=0
    _PVZ_BETA_LEAK_SUPPORT
)

if(USE_QT_UI)
    target_compile_definitions(pvztoolkit PRIVATE USE_QT_UI)
endif()

target_compile_options(
    pvztoolkit PRIVATE
    /Zc:wchar_t /Zc:__cplusplus
    /utf-8
)

if(USE_QT_UI)
    target_link_libraries(
        pvztoolkit PRIVATE
        Qt6::Core Qt6::Widgets
        crypt32.lib advapi32.lib wintrust.lib shlwapi.lib
    )
else()
    target_link_libraries(
        pvztoolkit PRIVATE
        fltk::fltk fltk::images fltk::jpeg fltk::png fltk::z
        crypt32.lib advapi32.lib wintrust.lib shlwapi.lib
    )
endif()
