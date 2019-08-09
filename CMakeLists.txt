cmake_minimum_required(VERSION 3.13)
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
        textEditor/utils/Character.cpp textEditor/utils/Character.h
        textEditor/utils/Pos.cpp textEditor/utils/Pos.h
        textEditor/utils/Identifier.cpp textEditor/utils/Identifier.h
        textEditor/utils/Version.cpp textEditor/utils/Version.h
        textEditor/utils/VersionVector.cpp textEditor/utils/VersionVector.h textEditor/Controller.cpp textEditor/Controller.h)

target_link_libraries(textEditor ${LIBRARIES})