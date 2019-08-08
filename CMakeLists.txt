cmake_minimum_required(VERSION 3.14)
project(textEditor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Charts REQUIRED)

list(APPEND LIBRARIES
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Charts)
list(REMOVE_DUPLICATES LIBRARIES)

add_executable(textEditor
        textEditor/textEditor.qrc
        textEditor/main.cpp
        textEditor/Editor.cpp textEditor/Editor.h
        textEditor/CRDT.cpp textEditor/CRDT.h
        textEditor/Character.cpp textEditor/Character.h
        textEditor/Pos.cpp textEditor/Pos.h
        textEditor/Identifier.cpp textEditor/Identifier.h
        textEditor/Version.cpp textEditor/Version.h
        textEditor/VersionVector.cpp textEditor/VersionVector.h)

target_link_libraries(textEditor ${LIBRARIES})