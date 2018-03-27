#
# MidiEditor
# Copyright (C) 2010  Markus Schwenk
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required(VERSION 3.5.0)

project(MidiEditor CXX)

message(STATUS "Running CMake for project MidiEditor...")
message(STATUS "Source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}")

configure_file(
    "${CMAKE_SOURCE_DIR}/version.cmake.in"
    "${CMAKE_BINARY_DIR}/version.cmake"
    )

option(INSTALL_WITH_CPACK "Make deployment on Windows installer or Linux DEB file" OFF)
option(ENABLE_REMOTE "Enable Android remote control" OFF)
option(ENABLE_GBA "Enable utilities for Game Boy Advance music hacking" OFF)
option(DISABLE_BIT_PACK "Disable most bitfields. This can have a very slight speed improvement at the cost of slightly more memory usage." OFF)

message(STATUS "Checking your compiler...")

if (WIN32)

    option(NO_CONSOLE_MODE "Start MidiEditor in a console window for debugging" OFF)

    if (MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()

endif()

if(ARCH STREQUAL "x64" )
    add_definitions(-D__ARCH64__)
endif()

# Turn on automatic invocation of the MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

message(STATUS "Checking Qt version...")

# We use QUIET because we want to show a more helpful error message.
find_package(Qt5 COMPONENTS Core Widgets Xml Multimedia QUIET HINTS $ENV{QTDIR} ${Qt5_DIR} ${CMAKE_PREFIX_PATH} ${QT5_DIR_GUESS})

if (ENABLE_REMOTE)
    find_package(Qt5 COMPONENTS Network QUIET HINTS $ENV{QTDIR} ${Qt5_DIR} ${CMAKE_PREFIX_PATH} ${QT5_DIR_GUESS})
endif()

if (APPLE)
    find_package(Qt5 COMPONENTS MacExtras QUIET HINTS $ENV{QTDIR} ${Qt5_DIR} ${CMAKE_PREFIX_PATH} ${QT5_DIR_GUESS})
endif()


# Check Qt version
if (Qt5Core_FOUND)
    if (Qt5Core_VERSION VERSION_LESS 5.9)
        message(FATAL_ERROR "Your Qt version (${Qt5Core_VERSION}) is out of date.\
            Please update to Qt >= 5.9.")
    else()
        message(STATUS "Found Qt version ${Qt5Core_VERSION}.")
    endif()
else()
    message(FATAL_ERROR " Couldn't find an installation of Qt >= 5.9.\n\
        \
        Please set the environment variable QTDIR to the path to your Qt 5 installation. This should be where you can find the bin folder containing qmake.\n\
        \
        Examples:
        C:/Qt/5.9.1/msvc2015_64\n\
        \
        /opt/qt\n\
        \
        ~/Qt/5.9.1/clang_64\n\
        \
        Please note that if you import this project into a properly configured Qt Creator, it will set this automatically.")
endif()

message(STATUS "Collecting files…")

# TODO: Globbing is evil.
file( GLOB main      src/*.cpp            )
file( GLOB gui       src/gui/*.cpp        )
file( GLOB MidiEvent src/MidiEvent/*.cpp  )
file( GLOB Protocol  src/protocol/*.cpp   )
file( GLOB Midi      src/midi/*.cpp       )
file( GLOB RTMidi    src/midi/rtmidi/*.cpp)
if (ENABLE_GBA)
file( GLOB gba       src/gba/*.cpp        )
endif()
if(ENABLE_REMOTE)
    file( GLOB Remote    src/remote/*.cpp  )
endif()
file( GLOB Tool      src/tool/*.cpp       )

file( GLOB mainh      src/*.h             )
file( GLOB guih       src/gui/*.h         )
file( GLOB MidiEventh src/MidiEvent/*.h   )
file( GLOB Protocolh  src/protocol/*.h    )
file( GLOB Midih      src/midi/*.h        )
file( GLOB RTMidih    src/midi/rtmidi/*.h )
if (ENABLE_GBA)
    file( GLOB gbah       src/gba/*.h         )
endif()
if (ENABLE_REMOTE)
    file( GLOB Remoteh    src/remote/*.h     )
endif()
file( GLOB Toolh      src/tool/*.h       )
set( versionh   ${CMAKE_BINARY_DIR}/version.h )

source_group( "gui"            FILES ${gui}       )
source_group( "gui\\inc"       FILES ${guih}      )
source_group( "MidiEvent"      FILES ${MidiEvent} )
source_group( "MidiEvent\\inc" FILES ${MidiEventh})
source_group( "protocol"       FILES ${Protocol}  )
source_group( "protocol\\inc"  FILES ${Protocolh} )
source_group( "midi"           FILES ${Midi}      )
source_group( "midi\\inc"      FILES ${Midih}     )
if (ENABLE_GBA)
    source_group( "gba"            FILES ${gba}   )
    source_group( "gba\\inc"       FILES ${gbah}  )
endif()
if (ENABLE_REMOTE)
    source_group( "remote"         FILES ${Remote}    )
    source_group( "remote\\inc"    FILES ${Remoteh}   )
endif()
source_group( "tool"           FILES ${Tool}      )
source_group( "tool\\inc"      FILES ${Toolh}     )
source_group( ""               FILES ${main} ${mainh} ${versionh})
source_group( "midi\\rtmidi"   FILES ${RTMidi} ${RTMidih} )

# TODO: AutoRCC? It seems to work.
include("ressources.cmake")

include("${CMAKE_CURRENT_BINARY_DIR}/version.cmake")

add_executable(MidiEditor midieditor.rc)
target_sources(MidiEditor PRIVATE
    ${main} ${mainh}
    ${gui}  ${guih}
    ${Tool} ${Toolh}
    ${MidiEvent} ${MidiEventh}
    ${Midi} ${Midih}
    ${RTMidi} ${RTMidih}
    ${Protocol} ${Protocolh})
target_include_directories(MidiEditor PRIVATE "${CMAKE_BINARY_DIR}")

if (ENABLE_GBA)
    target_sources(MidiEditor PRIVATE ${gba} ${gbah})
    target_compile_definitions(MidiEditor PRIVATE ENABLE_GBA)
endif()
if (ENABLE_REMOTE)
    target_sources(MidiEditor PRIVATE ${Remote} ${Remoteh})
    target_compile_definitions(MidiEditor PRIVATE ENABLE_REMOTE)
endif()
if (DISABLE_BIT_PACK)
    target_compile_definitions(MidiEditor PRIVATE NO_BIT_PACK)
endif()

if (APPLE)
    set(CMAKE_MACOSX_RPATH ON)
    option(USE_JACK "Use Jack Audio as well as CoreAudio for IO" OFF)

    # Generate Info.plist manually because we can get file association.
    set(BUNDLE_INFO_STRING "MidiEditor")
    set(BUNDLE_GUI_IDENTIFIER "com.abreheret.MidiEditor")
    set(BUNDLE_BUNDLE_NAME "MidiEditor")
    set(BUNDLE_ICON_FILE "MidiEditor.icns")
    set(BUNDLE_COPYRIGHT "© 2010-2018 Markus Schwenk")
    set(BUNDLE_SHORT_VERSION_STRING "3.1.1")
    set(BUNDLE_BUNDLE_VERSION "3.1.1")
    set(BUNDLE_LONG_VERSION_STRING "${GIT_TAG}")
    configure_file("${CMAKE_SOURCE_DIR}/Info.plist.cmake" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    set_target_properties(MidiEditor PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")

    target_sources(MidiEditor PRIVATE run_environment/MidiEditor.icns "${CMAKE_CURRENT_BINARY_DIR}/ressources.rcc")

    set_target_properties(MidiEditor PROPERTIES MACOSX_BUNDLE YES)
    set_target_properties(MidiEditor PROPERTIES RESOURCE "${CMAKE_CURRENT_BINARY_DIR}/ressources.rcc")
    set_source_files_properties(run_environment/MidiEditor.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/ressources.rcc PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
elseif(WIN32 AND NO_CONSOLE_MODE)
    set_target_properties(MidiEditor PROPERTIES WIN32_EXECUTABLE YES )
endif()

target_link_libraries(MidiEditor Qt5::Widgets Qt5::Xml Qt5::Multimedia)
if (ENABLE_REMOTE)
    target_link_libraries(MidiEditor Qt5::Network)
endif()
if (APPLE)
    target_link_libraries(MidiEditor Qt5::MacExtras)
endif()


message(STATUS "Linking libraries...")
if(WIN32)
    #	target_link_libraries(MidiEditor Qt:WinMain)
    #	if(NO_CONSOLE_MODE)
    #		if (MINGW)
    #			set_target_properties(MidiEditor PROPERTIES LINK_FLAGS "-mwindows")
    #		else()
    #			set_target_properties(MidiEditor PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    #		endif()
    #	endif()

    target_sources(MidiEditor PRIVATE
        "run_environment/MidiEditor.ico")

    set(QT5_DLLs_DEP_RELEASE "${Qt5_DIR}/../../../bin/Qt5Widgets.dll"
        "${Qt5_DIR}/../../../bin/Qt5Network.dll"
        "${Qt5_DIR}/../../../bin/Qt5Xml.dll"
        "${Qt5_DIR}/../../../bin/Qt5Gui.dll"
        "${Qt5_DIR}/../../../bin/Qt5Core.dll"
        "${Qt5_DIR}/../../../bin/Qt5Multimedia.dll")
    set(QT5_DLLs_DEP_DEBUG   "${Qt5_DIR}/../../../bin/Qt5Widgetsd.dll"
        "${Qt5_DIR}/../../../bin/Qt5Networkd.dll"
        "${Qt5_DIR}/../../../bin/Qt5Xmld.dll"
        "${Qt5_DIR}/../../../bin/Qt5Guid.dll"
        "${Qt5_DIR}/../../../bin/Qt5Cored.dll"
        "${Qt5_DIR}/../../../bin/Qt5Multimediad.dll")
    file(COPY ${QT5_DLLs_DEP_RELEASE} ${QT5_DLLs_DEP_DEBUG} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(MidiEditor winmm.lib)
    target_compile_definitions(MidiEditor PRIVATE __WINDOWS_MM__)
    if (MSVC)
        target_compile_definitions(MidiEditor PRIVATE _UNICODE UNICODE _MBCS)
    endif()
elseif(APPLE)
    message(STATUS "Setting up macOS frameworks...")
    #   include_directories(/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks )
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(CORE_MIDI CoreMIDI )
    find_library(CORE_AUDIO CoreAudio )
    mark_as_advanced (CORE_FOUNDATION CORE_MIDI CORE_AUDIO)
    set(EXTRA_LIBS ${CORE_FOUNDATION} ${CORE_AUDIO} ${CORE_MIDI} )
    target_link_libraries(MidiEditor ${EXTRA_LIBS})
    target_compile_definitions(MidiEditor PRIVATE __MACOSX_CORE__)
    if (USE_JACK)
        target_include_directories(MidiEditor PRIVATE /usr/local/lib)
        link_directories(/usr/local/lib)
        target_compile_definitions(MidiEditor PRIVATE __UNIX_JACK__)
        target_link_libraries(MidiEditor jack)
    endif()
else()
    target_compile_definitions(MidiEditor PRIVATE __LINUX_ALSASEQ__ __LINUX_ALSA__)
    target_link_libraries(MidiEditor asound sfml-system sfml-audio pthread)
    if (USE_JACK)
        target_compile_definitions(MidiEditor PRIVATE __UNIX_JACK__)
        target_link_libraries(MidiEditor jack)
    endif()
endif()

# Disable RTTI (dynamic_cast and typeid). We no longer need this, and this has the benefit
# of better performance and smaller executables.
if (MSVC)
    target_compile_options(MidiEditor PRIVATE "/GR-")
else()
    target_compile_options(MidiEditor PRIVATE "-fno-rtti")
endif()

# Throw an error about those sneaky QString allocations.
#
# For the reference, if you do stuff like this,
#     QString str = "Hello";
#     hai += " world!";
#     QLabel("Hai world"); // takes QString as an argument
# it is more like...
#     QString str = QString("Hello");
#     hai += QString(" world!");
#     QLabel(QString("Hai world"));
# and QString(const char *str) does this:
# {
#     QString s = QString::fromUtf8(str, strlen(str));
#     ref = s.ref();
# }
target_compile_definitions(MidiEditor PRIVATE QT_RESTRICTED_CAST_FROM_ASCII QT_USE_FAST_OPERATOR_PLUS)
if (MSVC)
    # TODO: Check to make sure this is the right code.
    target_compile_options(MidiEditor PRIVATE "/we4996")
else()
    target_compile_options(MidiEditor PRIVATE "-Werror=deprecated-declarations")
endif()

#file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/run_environment/midieditor.ico" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/run_environment/metronome/metronome-01.wav" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/metronome)

if(INSTALL_WITH_CPACK)
    message(STATUS "Configuring CPack...")
    install(TARGETS MidiEditor DESTINATION bin)
    install("${CMAKE_CURRENT_BINARY_DIR}/ressources.rcc" DESTINATION bin)
    install("${CMAKE_CURRENT_SOURCE_DIR}/run_environment/metronome/metronome-01.wav" DESTINATION bin/metronome)
    if(WIN32) #Windows
        install(${QT5_DLLs_DEP_RELEASE} DESTINATION bin)
        set(CPACK_GENERATOR NSIS)
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "MidiEditor")
        set(CPACK_PACKAGE_EXECUTABLES "MidiEditor" "MidiEditor" ${CPACK_PACKAGE_EXECUTABLES})
        set(CPACK_NSIS_MODIFY_PATH OFF)

        # TODO: macOS

    else() #UNIX
        set(CPACK_GENERATOR "DEB")
        SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Amaury Bréhéret")
        if (USE_JACK)
            set (CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5widgets5:amd64 (>=5.9.1), libqt5sound5:amd64 (>=5.9.1), libqt5network5:amd64 (>=5.9.1), libqt5xml:amd64 (>=5.9.1), libsfml-system,libsfml-audio,libasound,libpthread,jack2")
        else()
            set (CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5widgets5:amd64 (>=5.9.1), libqt5sound5:amd64 (>=5.9.1), libqt5network5:amd64 (>=5.9.1), libqt5xml:amd64 (>=5.9.1), libsfml-system,libsfml-audio,libasound,libpthread")
        endif()
        #set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt5-default (>= 5.9.1),libsfml-system,libsfml-audio,libasound,libpthread")
        #set(CPACK_DEBIAN_PACKAGE_DEPENDS "libQt5Widgets,libQt5Sound,libQt5Network,libQt5Xml,libQt5Core,libsfml-system,libsfml-audio,libasound,libpthread")
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    endif()
    set(CPACK_PACKAGE_CONTACT "Amaury Bréhéret abreheret@gmail.com")
    set(CPACK_PACKAGE_NAME "MidiEditor")
    set(CPACK_PACKAGE_VENDOR "https://github.com/abreheret/MidiEditor")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MidiEditor - Installation ")
    set(CPACK_PACKAGE_VERSION "3.1.1")
    set(CPACK_PACKAGE_VERSION_MAJOR "3")
    set(CPACK_PACKAGE_VERSION_MINOR "1")
    set(CPACK_PACKAGE_VERSION_PATCH "1")
    include(CPack)
endif()
