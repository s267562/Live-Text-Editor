#include "Messanger.h"
#include <QPixmap>
#include <QJsonDocument>
#include <QtCore/QBuffer>

Q_DECLARE_METATYPE(Message);

Messanger::Messanger(QObject *parent) : QObject(parent) {
    this->socket = new QTcpSocket(this);
    /* define initial state */
    reciveOkMessage = false;
    state = UNLOGGED;
    clientIsLogged = false;
    clientIsDisconnected = false;
    qRegisterMetaType<Message>("Message");

    /* define connection */
    this->connectReadyRead = connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    this->connectDisconnected = connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}

/**
 * Method for connecting with server
 * @param host
 * @param port
 * @return result of connection
 */
bool Messanger::connectTo(QString host, QString port){
    if (clientIsDisconnected){
        this->socket = new QTcpSocket(this);
        reciveOkMessage = false;
        state = UNLOGGED;
        clientIsLogged = false;
        clientIsDisconnected = false;
        qRegisterMetaType<Message>("Message");

        /* define connection */
        this->connectReadyRead = connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        this->connectDisconnected = connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    }
    serverIP = host;
    serverPort = port;
    socket->connectToHost(host, port.toInt());
    int i = 0;

    while (i < 5) {                                     /* retry for 5 times */
        if (socket->waitForConnected(TIMEOUT)) {
            break;
        }
        if (i == 4){
            emit errorConnection();
            return false;
        }
        i++;
    }

    socketDescriptor = socket->socketDescriptor();

    qDebug() << "Messanger.cpp - connectTo()     " << socket->socketDescriptor() << " connected";
    qDebug() << ""; // newLine

    return true;
}

/**
 * this method allows to start reading from socket and calls different methods
 * that handles different type of message.
 */
void Messanger::onReadyRead(){
    if (socket->bytesAvailable() == 0){
        return;
    }

    QByteArray datas;
    if (!readChunck(socket, datas, 5)){
        return;
    }

    qDebug() << "Messanger.cpp - onReadyRead()     msg received:" << datas;
    qDebug() << ""; // newLine

    if (!clientIsLogged){
        if (state == UNLOGGED && datas.toStdString() == AVATAR_MESSAGE){
            if (!readUser()){
                return;
            }
            state = WAITING_LIST_OF_FILE;
        }else if (state == WAITING_LIST_OF_FILE && datas.toStdString() == LIST_OF_FILE){
            if (!readFileNames()){
                return;
            }
            clientIsLogged = true;
            state = LIST_OF_FILE_RECIVED;
            #if !UI
                        requestForFile("prova");         /* TEST: TEXT EDITOR */
            #endif
        }else if (datas.toStdString() == ERR_MESSAGE){
            if (!readError()){
                return;
            }
        }
    }else{
        if (state == LIST_OF_FILE_RECIVED && datas.toStdString() == AVATAR_MESSAGE){
            if (!readUser()){
                return;
            }
        }else if (datas.toStdString() == SENDING_FILE){
            if (!readFile()){
                return;
            }
            state = WAITING_LIST_OF_ONLINE_USERS;
        }else if (state == WAITING_LIST_OF_ONLINE_USERS && datas.toStdString() == LIST_OF_USERS){
            if (!readUsernames()){
                return;
            }
            state = EDIT_FILE_STATE;
        }else if (state == EDIT_FILE_STATE && datas.toStdString() == INSERT_MESSAGE){
            if (!readInsert()){
                return;
            }
        }else if (state == EDIT_FILE_STATE && datas.toStdString() == DELETE_MESSAGE){
            if (!readDelete()){
                return;
            }
        }else if (state == EDIT_FILE_STATE && datas.toStdString() == REMOVE_USER){
            if (!readRemoveUser()){
                return;
            }
        }else if (state == EDIT_FILE_STATE && datas.toStdString() == OK_MESSAGE){
            reciveOkMessage = true;
        }else if (state == EDIT_FILE_STATE && datas.toStdString() == LIST_OF_USERS){
            if (!readUsernames()){
                return;
            }
        }else if (state == EDIT_FILE_STATE && datas.toStdString() == STYLE_CAHNGED_MESSAGE){
            if (!readStyleChanged()){
                return;
            }
        }else if (state == EDIT_FILE_STATE && datas.toStdString() == AVATAR_MESSAGE){
            if (!readUser()){
                return;
            }
        }else  if (datas.toStdString() == ADD_FILE){
            if (!readAddFile()){
                return;
            }
        }
        else if(state == EDIT_FILE_STATE && datas.toStdString() == ALIGNMENT_CHANGED_MESSAGE) {
            if (!readAlignmentChanged()){
                return;
            }
        }
        else if (datas.toStdString() == ERR_MESSAGE){
            if (!readError()){
                return;
            }
        }else if (datas.toStdString() == USERNAME_LIST_FOR_FILE){
            if (!readUsernameList()){
                return;
            }
        } else if (datas.toStdString() == LIST_OF_FILE){
            if (!readFileNames()){
                return;
            }
        }
    }

    despatchMessage();
    onReadyRead();
}

/**
 * This method handles different error's cases
 * @return result of reading from socket
 */
bool Messanger::readError(){
    qDebug() << "Messanger.cpp - readError()     ---------- READ ERROR ----------";
    QByteArray type;
    readSpace(socket);
    if (!readChunck(socket, type, 5)){
        return false;
    }

    if (type.toStdString() == LOGIN_MESSAGE){
        emit loginFailed();
    }else if (type.toStdString() == REGISTRATION_MESSAGE){
        emit registrationFailed();
    }else if (type.toStdString() == REQUEST_FILE_MESSAGE) {
        emit requestForFileFailed();
    }else if (type.toStdString() == INSERT_MESSAGE){
        emit insertFailed();
    }else if (type.toStdString() == DELETE_MESSAGE){
        emit deleteFailed();
    }else if (type.toStdString() == EDIT_ACCOUNT){
        emit editAccountFailed();
    }else if (type.toStdString() == SHARE_CODE){
        emit shareCodeFailed();
    }

    return true;
}

bool Messanger::despatchMessage(){
    if (!messages.empty()){
        QByteArray message = messages.front();
        messages.pop();
        if (!writeMessage(socket, message)){
            messages.push(message);
            return false;
        }
        reciveOkMessage = false;
    }else{
        reciveOkMessage = true;
    }
    return true;
}

/**
 * This method sands username and password to the server for login
 * @param username
 * @param password
 * @return result of writing on socket
 */
bool Messanger::logIn(QString username, QString password) {
    qDebug() << "Messanger.cpp - logIn()     ---------- LOGIN ----------";

    if (!clientIsLogged) {
        QByteArray message(LOGIN_MESSAGE);
        QByteArray usernameByteArray = convertionQString(username);
        QByteArray passwordByteArray = convertionQString(password);
        QByteArray usernameSize = convertionNumber(usernameByteArray.size());
        QByteArray passwordSize = convertionNumber(passwordByteArray.size());


        message.append(" " + usernameSize + " " + usernameByteArray  + " " + passwordSize + " " + passwordByteArray);
        qDebug() << "                        " << message;
        qDebug() << ""; // newLine

        if (!writeMessage(socket, message)){
            return false;
        }
    }
    return true;
}

/**
 * This method sands username and password to the server for registration
 * @param username
 * @param password
 * @return result of writing on socket
 */
bool Messanger::registration(QString username, QString password, QByteArray avatar){
    qDebug() << "Messanger.cpp - registration()     ---------- REGISTRATION ----------";

    QByteArray message(REGISTRATION_MESSAGE);

    QByteArray usernameByteArray = convertionQString(username);
    QByteArray passwordByteArray = convertionQString(password);
    QByteArray usernameSize = convertionNumber(usernameByteArray.size());
    QByteArray passwordSize = convertionNumber(passwordByteArray.size());
    QByteArray imageSize = convertionNumber(avatar.size());

    message.append(" " + usernameSize + " " + usernameByteArray + " " + passwordSize + " " + passwordByteArray + " " + imageSize + " " + avatar);

    qDebug() << "                                " << "username: " << username << " password: " << password << " avatarSize: " << avatar.size();
    qDebug() << ""; // newLine

    if (!writeMessage(socket, message)){
        return false;
    }

    return true;
}

/**
 * This methods sends user's changes to server
 * @param username
 * @param newPassword
 * @param oldPassword
 * @param avatar
 * @return result of writing on socket
 */
bool Messanger::sendEditAccount(QString username, QString newPassword, QString oldPassword, QByteArray avatar){
    qDebug() << "Messanger.cpp - sendEditAccount()     ---------- SEND EDIT ACCOUNT ----------";

    QByteArray message(EDIT_ACCOUNT);

    QByteArray usernameByteArray = convertionQString(username);
    QByteArray oldPasswordByteArray = convertionQString(oldPassword);
    QByteArray newPasswordByteArray = convertionQString(newPassword);

    QByteArray usernameSize = convertionNumber(usernameByteArray.size());
    QByteArray newPasswordSize = convertionNumber(newPasswordByteArray.size());
    QByteArray oldPasswordSize = convertionNumber(oldPasswordByteArray.size());
    QByteArray imageSize = convertionNumber(avatar.size());

    message.append(" " + usernameSize + " " + usernameByteArray + " " + newPasswordSize + " " + newPasswordByteArray + " "
    + oldPasswordSize + " " + oldPasswordByteArray + " " + imageSize + " " + avatar);

    qDebug() << "                                " << message;
    qDebug() << ""; // newLine

    if (!writeMessage(socket, message)){
        return false;
    }

    return true;
}

/**
 * This methods recives user's features
 * @return  result of reading from socket
 */
bool Messanger::readUser() {
    qDebug() << "Messanger.cpp - readUser()     ---------- READUSER ----------";
    readSpace(socket);
    int usernameSize = readNumberFromSocket(socket);

    readSpace(socket);

    QString username;
    if (!readQString(socket, username, usernameSize)) {
        return false;
    }

    readSpace(socket);
    int imageSize = readNumberFromSocket(socket);

    qDebug() << imageSize;

    readSpace(socket);
    QByteArray avatar;

    if (!readChunck(socket, avatar, imageSize)) {
        return false;
    }

    QImage image;
    QPixmap pixmap;
    if (imageSize != 0) {
        image = QImage::fromData(avatar, "PNG");
        pixmap = QPixmap::fromImage(image);
    }
    User *user = new User(username, pixmap);

    emit reciveUser(user);
    return true;
}

/**
 * This method reads the list of file name.
 * @return result of reading from socket
 */
bool Messanger::readFileNames(){
    qDebug() << "Messanger.cpp - readFileName()     ---------- READ FILE NAME ----------";
    QStringList fileList;
    std::map<QString, bool> files; // pair : (filename , ownership)

    readSpace(socket);
    int numFiles = readNumberFromSocket(socket);

    qDebug() << "                                numFiles: " << numFiles;

    for (int i = 0; i < numFiles; i++){
        readSpace(socket);
        int fileNameSize = readNumberFromSocket(socket);
        readSpace(socket);
        QString fileName;
        if (!readQString(socket, fileName, fileNameSize)){
            return false;
        }
        readSpace(socket);
        QByteArray flag;
        if (!readChunck(socket, flag, 1)){
            return false;
        }

        bool flagBool = flag.toInt() == 1;
        files[fileName] = flagBool;

        qDebug() << "                               " << i + 1 << "." << fileName;
    }

    qDebug() << ""; // newLine

    emit fileNames(files);

    return true;
}

/**
 * This method allows logout feature
 * @return result of connectTo
 */
bool Messanger::logOut(){
    if (clientIsLogged){
        disconnect(connectReadyRead);
        disconnect(connectDisconnected);
        socket->deleteLater();

        socket = new QTcpSocket(this);
        if (!connectTo(serverIP, serverPort)){
            qDebug() << "Connesione fallita";
            return false;
        }

        clientIsLogged = false;
        state = UNLOGGED;
        reciveOkMessage = false;
        connectReadyRead = connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        connectDisconnected = connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
        emit logout();
    }
    return true;
}

/**
 * This method sends a request for specific file
 * @param fileName
 * @return result of writing on socket
 */
bool Messanger::requestForFile(QString fileName){
    qDebug() << "Messanger.cpp - requestForFile()     ---------- REQUEST FOR FILE ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(REQUEST_FILE_MESSAGE);
        QByteArray fileNameByteArray = convertionQString(fileName);
        QByteArray fileNameSize = convertionNumber(fileNameByteArray.size());

        message.append(" " + fileNameSize + " " + fileNameByteArray);
        qDebug() << "                                 " << message;
        qDebug() << ""; // newLine


        reciveOkMessage = true;
        if (!writeMessage(socket, message)){
            return false;
        }

        state = WAITING_LIST_OF_ONLINE_USERS;
        return true;
    }else{
        return false;
    }
}

/**
 * This method reads the list of online users
 * @return result of reading from socket
 */
bool Messanger::readUsernames(){
    qDebug() << "Messanger.cpp - readUsernames()     ---------- READ USERNAMES ----------";
    QStringList usernames;
    readSpace(socket);
    int usernamesSize = readNumberFromSocket(socket);

    qDebug() << usernamesSize;

    if (usernamesSize != 0){
        for (int i = 0; i < usernamesSize; i++){
            readSpace(socket);
            int usernameSize = readNumberFromSocket(socket);
            qDebug() << usernameSize;
            readSpace(socket);
            QString username;
            if (!readQString(socket, username, usernameSize)){
                return false;
            }
            qDebug() << "                              usename: "<<username;
            usernames.append(username);
        }
    }

    emit setUsers(usernames);
    return true;
}

/**
 * This method reads the username that is not online
 * @return result of reading from socket
 */
bool Messanger::readRemoveUser(){
    qDebug() << "Messanger.cpp - readRemoveUser()     ---------- READ REMOVE USER ----------";
    readSpace(socket);
    int usernameSize = readNumberFromSocket(socket);
    readSpace(socket);

    QString username;
    if (!readQString(socket, username, usernameSize)){
        return false;
    }

    emit removeUser(username);
    return true;
}

/**
 * This method receives the complite file, that was requested
 * @return result of reading from socket
 */
bool Messanger::readFile(){
    qDebug() << "Messanger.cpp - readFile()     ---------- READ FILE ----------";
    std::vector<std::vector<Character>> file;
    readSpace(socket);
    int filenameSize = readNumberFromSocket(socket);
    readSpace(socket);
    QString filename;
    if (!readQString(socket, filename, filenameSize)){
        return false;
    }
    readSpace(socket);
    int numLines = readNumberFromSocket(socket);

    for (int i = 0; i < numLines; i++){
        std::vector<Character> line;
        readSpace(socket);
        int numCharacters = readNumberFromSocket(socket);

        for (int j = 0; j < numCharacters; j++){
            readSpace(socket);
            int messageSize = readNumberFromSocket(socket);
            readSpace(socket);

            QByteArray characterByteFormat;
            if (!readChunck(socket, characterByteFormat, messageSize)){
                return false;
            }

            QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
            Character character = Character::toCharacter(jsonDocument);

            line.push_back(character);
        }
        file.push_back(line);
        writeOkMessage(socket);
    }

    std::vector<std::pair<Character,int>> styleBlocks;

    readSpace(socket);
    int numBlocks = readNumberFromSocket(socket);

    QByteArray characterByteFormat;

    for (int j = 0; j < numBlocks; j++){
        readSpace(socket);
        int messageSize = readNumberFromSocket(socket);
        readSpace(socket);

        if (!readChunck(socket, characterByteFormat, messageSize)){
            return false;
        }

        QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
        Character character = Character::toCharacter(jsonDocument);

        readSpace(socket);
        int alignment = readNumberFromSocket(socket);
        styleBlocks.emplace_back(character,alignment);
        writeOkMessage(socket);
    }

    emit fileRecive(file, styleBlocks, filename);
    return true;
}

/**
 * This method sends the character, that was added
 * @param character
 * @return result of writing on socket
 */
bool Messanger::writeInsert(Character character){
    qDebug() << "Messanger.cpp - writeInsert()     ---------- WRITE INSERT ----------";
    qDebug() << "Messanger: "<< QThread::currentThreadId();

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(INSERT_MESSAGE);
        QByteArray characterByteFormat = character.toQByteArray();
        QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

        message.append(" " + sizeOfMessage + " " + characterByteFormat);
        messages.push(message);

        qDebug() << "                         " << message;
        qDebug() << ""; // newLine

        if (reciveOkMessage){
            reciveOkMessage = false;
            if (!writeMessage(socket, message)){
                return false;
            }
            messages.pop();
        }
    }
    return true;
}

/**
 * This method sends the character with another style
 * @param character
 * @return result of writing on socket
 */
bool Messanger::writeStyleChanged(Character character){
    qDebug() << "Messanger.cpp - writeStyleChanged()     ---------- WRITE STYLE CHANGED ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(STYLE_CAHNGED_MESSAGE);
        QByteArray characterByteFormat = character.toQByteArray();
        QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

        message.append(" " + sizeOfMessage + " " + characterByteFormat);
        messages.push(message);

        qDebug() << "                         " << message;
        qDebug() << ""; // newLine

        if (reciveOkMessage){
            reciveOkMessage = false;
            if (!writeMessage(socket, message)){
                return false;
            }
            messages.pop();
        }
    }
    return true;
}

/**
 * This method sends the alignment changes
 * @param alignment_type
 * @param blockId
 * @return
 */
bool Messanger::writeAlignmentChanged(int alignment_type, Character blockId){
    qDebug() << "Messanger.cpp - writeAlignmentChanged()     ---------- WRITE ALIGNMENT CHANGED ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(ALIGNMENT_CHANGED_MESSAGE);
        QByteArray alignmentByteFormat=convertionNumber(alignment_type);
        QByteArray blockIdByteFormat=blockId.toQByteArray();
        //QByteArray sizeOfMessageAt = convertionNumber(alignmentByteFormat.size());
        QByteArray sizeBlockId = convertionNumber(blockIdByteFormat.size());


        message.append(" " + alignmentByteFormat);
        message.append(" " + sizeBlockId);
        message.append(" " + blockIdByteFormat);
        messages.push(message);

        qDebug() << "                         " << message;
        qDebug() << ""; // newLine

        if (reciveOkMessage){
            reciveOkMessage = false;
            if (!writeMessage(socket, message)){
                return false;
            }
            messages.pop();
        }
    }
    return true;
}

/**
 * This method sends the character, that was removed
 * @param character
 * @return result of writing on socket
 */
bool Messanger::writeDelete(Character character){
    qDebug() << "Messanger.cpp - writeDelete()     ---------- WRITE DELETE ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(DELETE_MESSAGE);
        QByteArray characterByteFormat = character.toQByteArray();
        QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

        message.append(" " + sizeOfMessage + " " + characterByteFormat);
        messages.push(message);
        if (reciveOkMessage) {
            reciveOkMessage = false;

            if (!writeMessage(socket, message)) {
                return false;
            }
            messages.pop();

        }
    }
    return true;
}

/**
 * This method reads the character, that was added from other users
 * @return result of reading from socket
 */
bool Messanger::readInsert(){
    qDebug() << "Messanger.cpp - readInsert()     ---------- READ INSERT ----------";

    readSpace(socket);
    int messageSize = readNumberFromSocket(socket);
    readSpace(socket);

    QByteArray characterByteFormat;
    if (!readChunck(socket, characterByteFormat, messageSize)){
        return false;
    }

    readSpace(socket);
    int sizeOfsender = readNumberFromSocket(socket);
    readSpace(socket);

    QByteArray usernameByteFormat;
    if (!readChunck(socket, usernameByteFormat, sizeOfsender)){
        return false;
    }

    QString username(usernameByteFormat);

    qDebug() << "Read username: " << username;

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    Message message(character, socket->socketDescriptor(), INSERT, username);

    //emit newMessage(message);
    QMetaObject::invokeMethod(crdt, "newMessage", Qt::QueuedConnection, Q_ARG(Message, message));
    return true;
}

/**
 * This method reads the style changes
 * @return
 */
bool Messanger::readStyleChanged(){
    qDebug() << "Messanger.cpp - readStyleChanged()     ---------- READ STYLE CHANGED ----------";

    readSpace(socket);
    int messageSize = readNumberFromSocket(socket);
    readSpace(socket);

    QByteArray characterByteFormat;
    if (!readChunck(socket, characterByteFormat, messageSize)){
        return false;
    }

    readSpace(socket);
    int sizeOfsender = readNumberFromSocket(socket);
    readSpace(socket);

    QByteArray usernameByteFormat;
    if (!readChunck(socket, usernameByteFormat, sizeOfsender)){
        return false;
    }

    QString username(usernameByteFormat);

    qDebug() << "Read username: " << username;

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    Message message(character, socket->socketDescriptor(), STYLE_CHANGED, username);

    emit newMessage(message);
    return true;
}

/**
 * This method reads the alignment changes
 * @return
 */
bool Messanger::readAlignmentChanged(){

    qDebug() << "Messanger.cpp - readAlignmentChanged()     ---------- READ ALIGNMENT CHANGED ----------";

    readSpace(socket);
    int alignType = readNumberFromSocket(socket);
    readSpace(socket);

    int sizeBlockId = readNumberFromSocket(socket);

    readSpace(socket);
    
    QByteArray characterByteFormat;
    if (!readChunck(socket, characterByteFormat, sizeBlockId)){
        return false;
    }

    QJsonDocument jsonFormatBlockId = QJsonDocument::fromBinaryData(characterByteFormat);
    Character blockId = Character::toCharacter(jsonFormatBlockId);

    readSpace(socket);
    int sizeOfsender = readNumberFromSocket(socket);
    readSpace(socket);

    QByteArray usernameByteFormat;
    if (!readChunck(socket, usernameByteFormat, sizeOfsender)){
        return false;
    }

    QString username(usernameByteFormat);

    qDebug() << "Read username: " << username;

//TODO: Remove " "

    Message message(blockId, socket->socketDescriptor(), ALIGNMENT_CHANGED, username, alignType);

    QMetaObject::invokeMethod(crdt, "newMessage", Qt::QueuedConnection, Q_ARG(Message, message));
    return true;
}

/**
 * This method reads the character, that was removed from other users
 * @return result of reading from socket
 */
bool Messanger::readDelete(){
    qDebug() << "Messanger.cpp - readDelete()     ---------- READ DELETE ----------";

    readSpace(socket);
    int messageSize = readNumberFromSocket(socket);
    readSpace(socket);

    QByteArray characterByteFormat;
    if (!readChunck(socket, characterByteFormat, messageSize)){
        return false;
    }

    readSpace(socket);
    int sizeOfsender = readNumberFromSocket(socket);
    readSpace(socket);
    QByteArray usernameByteFormat;
    if (!readChunck(socket, usernameByteFormat, sizeOfsender)){
        return false;
    }

    QString username(usernameByteFormat);

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    Message message(character, socket->socketDescriptor(), DELETE, username);

    QMetaObject::invokeMethod(crdt, "newMessage", Qt::QueuedConnection, Q_ARG(Message, message));
    return true;
}

bool Messanger::sendShareCode(QString shareCode){
    qDebug() << "Messanger.cpp - sendShareCode()     ---------- SEND SHARECODE ----------";
    QByteArray message(SHARE_CODE);

    QByteArray shareCodeSize = convertionNumber(shareCode.size());

    message.append( " " + shareCodeSize + " " + shareCode.toUtf8());

    qDebug() << "                                " << message;
    qDebug() << ""; // newLine

    if (!writeMessage(socket, message)){
        return false;
    }

    return true;
}

/**
 * This method reads the file added
 * @return
 */
bool Messanger::readAddFile(){
    qDebug() << "Messanger.cpp - readAddFile()     ---------- READ ADDFILE ----------";

    std::map<QString, bool> files;                      // pair : (filename , ownership)
    readSpace(socket);
    int fileNameSize = readNumberFromSocket(socket);
    readSpace(socket);

    QString fileName;
    if (!readQString(socket, fileName, fileNameSize)){
        return false;
    }
    readSpace(socket);
    QByteArray flag;
    if (!readChunck(socket, flag, 1)){
        return false;
    }

    bool flagBool = flag.toInt() == 1;
    files[fileName] = flagBool;
    emit addFileNames(files);
    return true;
}

/**
 * This method is called when the socket disconnects
 */
void Messanger::onDisconnect(){
    qDebug() << "Messanger.cpp - onDisconnect()     " << socketDescriptor <<" Disconnected";
    qDebug() << ""; // newLine

    QTcpSocket soc;
    soc.setSocketDescriptor(socketDescriptor);
    soc.deleteLater();
    clientIsDisconnected = true;
    emit errorConnection();
}

/**
 * This method sends the request for the usernames list
 * @param fileName
 * @return
 */
bool Messanger::requestForUsernameList(QString fileName){
    qDebug() << "Messanger.cpp - requestForUsernameList()     ---------- REQUEST FOR USERNAME LIST ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(REQUEST_USERNAME_LIST_MESSAGE);
        QByteArray fileNameByteArray = convertionQString(fileName);
        QByteArray fileNameSize = convertionNumber(fileNameByteArray.size());

        message.append(" " + fileNameSize + " " + fileNameByteArray);
        qDebug() << "                                 " << message;
        qDebug() << ""; // newLine


        reciveOkMessage = true;
        if (!writeMessage(socket, message)){
            return false;
        }

        //state = WAITING_LIST_OF_ONLINE_USERS;
        return true;
    }else{
        return false;
    }
}

/**
 * This method reads the usernames list
 * @return
 */
bool Messanger::readUsernameList(){
    qDebug() << "Messanger.cpp - readUsernameList()     ---------- READ USERNAME LIST ----------";
    QStringList usernames;
    readSpace(socket);
    int usernamesSize = readNumberFromSocket(socket);

    qDebug() << usernamesSize;

    if (usernamesSize != 0){
        for (int i = 0; i < usernamesSize; i++){
            readSpace(socket);
            int usernameSize = readNumberFromSocket(socket);
            qDebug() << usernameSize;
            readSpace(socket);
            QString username;
            if (!readQString(socket, username, usernameSize)){
                return false;
            }
            qDebug() << "                              usename: "<<username;
            usernames.append(username);
        }
    }

    emit reciveUsernameList("", usernames);
    return true;
}

/**
 * This method sends the file information changes
 * @param oldFilename
 * @param newFilename
 * @param usernames
 * @return
 */
bool Messanger::sendFileInfomationChanges(QString oldFilename, QString newFilename, QStringList usernames){
    qDebug() << "Messanger.cpp - sendFileInfomationChanges()     ---------- SEND FILE INFORMATION CHANGES ----------";
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(FILE_INFORMATION_CHANGES);
        QByteArray oldFileNameByteArray = convertionQString(oldFilename);
        QByteArray oldFileNameSize = convertionNumber(oldFileNameByteArray.size());
        QByteArray newFileNameByteArray = convertionQString(newFilename);
        QByteArray newFileNameSize = convertionNumber(newFileNameByteArray.size());

        message.append(" " + oldFileNameSize + " " + oldFileNameByteArray + " " + newFileNameSize + " " + newFileNameByteArray);
        qDebug() << "                                 " << message;
        qDebug() << ""; // newLine

        int nUsers = usernames.size();
        QByteArray numUsers = convertionNumber(nUsers);

        message.append(" " + numUsers);

        for (QString username : usernames){
            QByteArray usernameQBytearray = convertionQString(username);
            QByteArray usernameSize = convertionNumber(usernameQBytearray.size());
            message.append(" " + usernameSize + " " + usernameQBytearray);
        }

        reciveOkMessage = true;
        if (!writeMessage(socket, message)){
            return false;
        }

        //state = WAITING_LIST_OF_ONLINE_USERS;
        return true;
    }else{
        return false;
    }
}

/**
 * This method sends the file that which should be eliminated
 * @param filename
 * @return
 */
bool Messanger::sendDeleteFile(QString filename){
    qDebug() << "Messanger.cpp - sendDeleteFile()     ---------- SEND DELETE FILE ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(DELETE_FILE);
        QByteArray fileNameByteArray = convertionQString(filename);
        QByteArray fileNameSize = convertionNumber(fileNameByteArray.size());

        message.append(" " + fileNameSize + " " + fileNameByteArray);
        qDebug() << "                                 " << message;
        qDebug() << ""; // newLine


        reciveOkMessage = true;
        if (!writeMessage(socket, message)){
            return false;
        }

        //state = WAITING_LIST_OF_ONLINE_USERS;
        return true;
    }else{
        return false;
    }
}

void Messanger::setCrdt(CRDT *crdt) {
    Messanger::crdt = crdt;
}
