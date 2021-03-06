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
find_package(Qt5PrintSupport REQUIRED)

list(APPEND LIBRARIES
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Charts
        Qt5::Sql
        Qt5::Network
        Qt5::PrintSupport)
list(REMOVE_DUPLICATES LIBRARIES)

include_directories (client/ui)

add_executable(client
        client/textEditor.qrc
        client/ui/login.ui
        client/ui/login.cpp
        client/main.cpp
        client/resource.qrc
        client/editor.ui
        client/editor.cpp client/editor.h
        client/CRDT.cpp client/CRDT.h
        client/utils/Character.cpp client/utils/Character.h
        client/utils/CharFormat.h
        client/utils/Pos.cpp client/utils/Pos.h
        client/utils/Identifier.cpp client/utils/Identifier.h
        client/Controller.cpp client/Controller.h
        client/Networking/Messanger.h
        client/Networking/Messanger.cpp
        client/utils/Constants.h
        client/ui/connection.cpp client/ui/connection.ui
        client/ui/registration.ui client/ui/registration.cpp client/ui/registration.h client/ui/resources.qrc
        client/ui/clickablelabel.cpp client/ui/clickablelabel.h
        client/ui/showFiles.cpp client/ui/showFiles.ui
        client/ui/customwidget.cpp client/ui/customwidget.ui
        client/ui/sharefile.cpp client/ui/sharefile.ui
        client/ui/editaccount.cpp client/ui/editaccount.ui
        client/ui/loading.cpp client/ui/loading.ui
        client/Networking/common/commonFunctions.h
        client/Networking/common/commonFunctions.cpp
        client/User.cpp client/User.h client/utils/CharFormat.h)

add_executable(client2
        client/textEditor.qrc
        client/ui/login.ui
        client/ui/login.cpp
        client/main2.cpp
        client/resource.qrc
        client/editor.ui
        client/editor.cpp client/editor.h
        client/CRDT.cpp client/CRDT.h
        client/utils/Character.cpp client/utils/Character.h
        client/utils/CharFormat.h
        client/utils/Pos.cpp client/utils/Pos.h
        client/utils/Identifier.cpp client/utils/Identifier.h
        client/Controller.cpp client/Controller.h
        client/Networking/Messanger.h
        client/Networking/Messanger.cpp
        client/utils/Constants.h
        client/ui/connection.cpp client/ui/connection.ui
        client/ui/registration.ui client/ui/registration.cpp client/ui/registration.h client/ui/resources.qrc
        client/ui/clickablelabel.cpp client/ui/clickablelabel.h
        client/ui/showFiles.cpp client/ui/showFiles.ui
        client/ui/customwidget.cpp client/ui/customwidget.ui
        client/ui/sharefile.cpp client/ui/sharefile.ui
        client/ui/editaccount.cpp client/ui/editaccount.ui
        client/ui/loading.cpp client/ui/loading.ui
        client/Networking/common/commonFunctions.h
        client/Networking/common/commonFunctions.cpp
        client/User.cpp client/User.h client/utils/CharFormat.h)

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
        client/utils/CharFormat.h
        client/utils/Pos.h
        client/utils/Pos.cpp
        server/Utils/Constants.h
        server/Networking/common/commonFunctions.h
        server/Networking/common/commonFunctions.cpp
        server/Networking/message/Message.h)


target_link_libraries(client ${LIBRARIES})
target_link_libraries(client2 ${LIBRARIES})
target_link_libraries(server ${LIBRARIES})