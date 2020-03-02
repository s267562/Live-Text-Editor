cmake_minimum_required(VERSION 3.10)
project(textEditor)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS "-mwindows") # Uncomment when project finished. execute application without terminal on windows
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
        client/ui/Login/login.ui
        client/ui/Login/login.cpp
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
        common/Constants.h
        client/ui/Connection/connection.cpp client/ui/Connection/connection.ui
        client/ui/Registration/registration.ui client/ui/Registration/registration.cpp client/ui/Registration/registration.h client/ui/resources.qrc
        client/ui/clickablelabel.cpp client/ui/clickablelabel.h
        client/ui/ShowFiles/showFiles.cpp client/ui/ShowFiles/showFiles.ui
        client/ui/CustomWidget/customwidget.cpp client/ui/CustomWidget/customwidget.ui
        client/ui/ShareFile/sharefile.cpp client/ui/ShareFile/sharefile.ui
        client/ui/CreateFile/createfile.cpp client/ui/CreateFile/createfile.ui
        client/ui/EditAccount/editaccount.cpp client/ui/EditAccount/editaccount.ui
        client/ui/Loading/loading.cpp client/ui/Loading/loading.ui
        common/commonFunctions.h
        common/commonFunctions.cpp
        client/User.cpp client/User.h client/utils/CharFormat.h server/SimpleCrypt/SimpleCrypt.cpp server/SimpleCrypt/SimpleCrypt.h
        client/ui/AddFile/addfile.ui client/ui/AddFile/addfile.cpp
        client/ui/FileInformation/fileinformation.ui client/ui/FileInformation/fileinformation.cpp
        client/ui/CustomWidgetUsername/customwidgetusername.ui client/ui/CustomWidgetUsername/customwidgetusername.cpp)

add_executable(client2
        client/textEditor.qrc
        client/ui/Login/login.ui
        client/ui/Login/login.cpp
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
        common/Constants.h
        client/ui/Connection/connection.cpp client/ui/Connection/connection.ui
        client/ui/Registration/registration.ui client/ui/Registration/registration.cpp client/ui/Registration/registration.h client/ui/resources.qrc
        client/ui/clickablelabel.cpp client/ui/clickablelabel.h
        client/ui/ShowFiles/showFiles.cpp client/ui/ShowFiles/showFiles.ui
        client/ui/CustomWidget/customwidget.cpp client/ui/CustomWidget/customwidget.ui
        client/ui/ShareFile/sharefile.cpp client/ui/ShareFile/sharefile.ui
        client/ui/CreateFile/createfile.cpp client/ui/CreateFile/createfile.ui
        client/ui/EditAccount/editaccount.cpp client/ui/EditAccount/editaccount.ui
        client/ui/Loading/loading.cpp client/ui/Loading/loading.ui
        common/commonFunctions.h
        common/commonFunctions.cpp
        client/User.cpp client/User.h client/utils/CharFormat.h server/SimpleCrypt/SimpleCrypt.cpp server/SimpleCrypt/SimpleCrypt.h
        client/ui/AddFile/addfile.ui client/ui/AddFile/addfile.cpp
        client/ui/FileInformation/fileinformation.ui client/ui/FileInformation/fileinformation.cpp
        client/ui/CustomWidgetUsername/customwidgetusername.ui client/ui/CustomWidgetUsername/customwidgetusername.cpp)

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
        common/Constants.h
        common/commonFunctions.h
        common/commonFunctions.cpp
        common/Message.h server/SimpleCrypt/SimpleCrypt.cpp server/SimpleCrypt/SimpleCrypt.h)

# Disable qDebug output
target_compile_definitions(server PUBLIC QT_NO_DEBUG_OUTPUT)
target_compile_definitions(client PUBLIC QT_NO_DEBUG_OUTPUT)
target_compile_definitions(client2 PUBLIC QT_NO_DEBUG_OUTPUT)


target_link_libraries(client ${LIBRARIES})
target_link_libraries(client2 ${LIBRARIES})
target_link_libraries(server ${LIBRARIES})