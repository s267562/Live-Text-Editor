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
        client/ui/login.ui
        client/ui/login.cpp
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
        client/utils/Constants.h client/MainWindow.cpp client/MainWindow.h
        client/MainWindow.cpp client/MainWindow.h
        client/ui/connection.cpp client/ui/connection.ui
        client/ui/registration.ui client/ui/registration.cpp client/ui/registration.h client/ui/resources.qrc
        client/ui/clickablelabel.cpp client/ui/clickablelabel.h
        client/ui/showFiles.cpp client/ui/showFiles.ui
        cmake-build-debug/client_autogen/include/ui_registration.h
        cmake-build-debug/client_autogen/include/ui/ui_login.h
        cmake-build-debug/client_autogen/include/ui_connection.h
        cmake-build-debug/client_autogen/include/ui_showFiles.h
        cmake-build-debug/client_autogen/include/ui_login.h
        )

add_executable(server
        server/main.cpp
        server/DB/Database.cpp
        server/DB/Database.h
        server/Networking/Server.cpp
        server/Networking/Server.h
        server/Networking/Thread.cpp
        server/Networking/Thread.h
        server/CRDT.cpp
        server/CRDT.h
        client/utils/Identifier.h
        client/utils/Identifier.cpp
        client/utils/Character.h
        client/utils/Character.cpp
        client/utils/Pos.h
        client/utils/Pos.cpp
        client/utils/Version.h
        client/utils/Version.cpp
        client/utils/VersionVector.h
        client/utils/VersionVector.cpp
        server/Utils/Constants.h
        server/Networking/common/commonFunctions.h
        server/Networking/common/commonFunctions.cpp
        server/Networking/message/Message.h)


target_link_libraries(client ${LIBRARIES})
target_link_libraries(server ${LIBRARIES})