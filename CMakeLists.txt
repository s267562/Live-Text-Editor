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
find_package(Qt5Sql REQUIRED)
find_package(Qt5Network REQUIRED)

list(APPEND LIBRARIES
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Charts
        Qt5::Sql
        Qt5::Network)
list(REMOVE_DUPLICATES LIBRARIES)

add_executable(client
        client/textEditor.qrc
        client/main.cpp
        client/Editor.cpp client/Editor.h
        client/CRDT.cpp client/CRDT.h
        client/utils/Character.cpp client/utils/Character.h
        client/utils/Pos.cpp client/utils/Pos.h
        client/utils/Identifier.cpp client/utils/Identifier.h
        client/utils/Version.cpp client/utils/Version.h
        client/utils/VersionVector.cpp client/utils/VersionVector.h client/Controller.cpp client/Controller.h
        client/Networking/Client.h
        client/Networking/Client.cpp
        client/utils/Constants.h)

add_executable(server
        server/main.cpp
        server/DB/Database.cpp
        server/DB/Database.h
        server/Networking/Server.cpp
        server/Networking/Server.h
        server/Networking/Thread.cpp
        server/Networking/Thread.h
        server/utils/Constants.h server/Networking/common/commonFunctions.h server/Networking/common/commonFunctions.cpp)

target_link_libraries(client ${LIBRARIES})
target_link_libraries(server ${LIBRARIES})