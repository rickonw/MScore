cmake_minimum_required(VERSION 2.8.4)

project(MScore)

set(CMAKE_BUILD_TYPE Release)

if (APPLE)
       set(CMAKE_MACOSX_RPATH ON)
endif (APPLE)

option(NO_DARMS_SUPPORT         "Disable Humdrum support"                      ON)
option(NO_PAE_SUPPORT           "Disable Plaine & Easie support"               OFF)
option(NO_HUMDRUM_SUPPORT       "Disable Humdrum support"                      OFF)
option(MUSICXML_DEFAULT_HUMDRUM "Enable MusicXML to Humdrum by default"        OFF)
option(BUILD_AS_LIBRARY         "Build MScore as library"                     OFF)

if (NO_HUMDRUM_SUPPORT AND MUSICXML_DEFAULT_HUMDRUM)
    message(SEND_ERROR "Default MusicXML to Humdrum cannot be enabled by default without Humdrum support")
endif()

include_directories(
    ../thirdparty/verovio/include
    ../thirdparty/verovio/include/midi
    ../thirdparty/verovio/include/hum
    ../thirdparty/verovio/include/pugi
    ../thirdparty/verovio/include/utf8
    ../thirdparty/verovio/include/vrv
	../thirdparty/wildmidi/include
	../thirdparty/libmei/include
)

if(MSVC)
    add_definitions(/W2)
    add_definitions(/wd4244)          # suppress warning of possible loss of precision
    add_definitions(-DNO_PAE_SUPPORT) # regex is working differently under Windows so PAE is not supported (yet)
    include_directories(../thirdparty/verovio/include/win32)
else()
    # jsonxx raises -Wdollar-in-identifier-extension
    add_definitions(-Wall -W -pedantic -Wno-unused-parameter -Wno-dollar-in-identifier-extension)
    add_definitions(-g)
    add_definitions(-std=c++11)
endif()

execute_process(
    COMMAND ../tools/get_git_commit.sh
    OUTPUT_VARIABLE GIT_COMMIT
)

if(NO_DARMS_SUPPORT)
    add_definitions(-DNO_DARMS_SUPPORT)
endif()

if(NO_PAE_SUPPORT)
    add_definitions(-DNO_PAE_SUPPORT)
endif()

#if(NO_HUMDRUM_SUPPORT)
#    add_definitions(-DNO_HUMDRUM_SUPPORT)
#else()
#    file(GLOB hum_SRC "../src/hum/*.cpp")
#    if (MUSICXML_DEFAULT_HUMDRUM)
#        add_definitions(-DMUSICXML_DEFAULT_HUMDRUM)
#    endif()
#endif()

file(GLOB MScore_SRC "../src/*.cpp")
#file(GLOB midi_SRC "../src/midi/*.cpp")

# Add header files to custom target, otherwise they are not shown in some IDEs (e.g. QtCreator)
file(GLOB_RECURSE LibFiles "../include/*.h")
add_custom_target(headers SOURCES ${LibFiles})

set(all_SRC
    ${MScore_SRC}
    ${hum_SRC}
    ${midi_SRC}
    #../src/json/jsonxx.cc
)

link_libraries(../thirdparty/wildmidi/wildmidi_dynamic winmm)
link_libraries(../thirdparty/verovio/verovio)

if (BUILD_AS_LIBRARY)
    message(STATUS "build MScore as library")
    add_library (
        MScore
        SHARED
        ${all_SRC}
    )
else()
    message(STATUS "build MScore as executable")
    add_executable (
        MScore
        #main.cpp
        ${all_SRC}
    )
endif()

install(
    TARGETS MScore
    DESTINATION /usr/local/bin
)
install(
    DIRECTORY ../data/
    DESTINATION share/MScore
    FILES_MATCHING PATTERN "*.xml" PATTERN "*.svg"
)
