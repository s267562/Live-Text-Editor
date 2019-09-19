#include "Messanger.h"
#include "../utils/Identifier.h"
#include "common/commonFunctions.h"
#include <QFile>
#include <QPixmap>
#include <iostream>
#include <QJsonDocument>
#include <QtCore/QBuffer>

Messanger::Messanger(QObject *parent) : QObject(parent) {
    this->socket = new QTcpSocket(this);
    reciveOkMessage = false;
    state = UNLOGGED;

    /* define connection */
    c = connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    d = connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}

void Messanger::setCRDT(CRDT *crdt) {
    this->crdt = crdt;
}

/**
 * Method for connecting with server
 * @param host
 * @param port
 * @return result of connection
 */
bool Messanger::connectTo(QString host, QString port){
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
    clientIsLogged = false;
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
    /*
    if ((datas.toStdString() == OK_MESSAGE || datas.toStdString() == LIST_OF_FILE) && !clientIsLogged){
        //clientIsLogged = true;
        reciveOkMessage = true;
    }else if (datas.toStdString() == ERR_MESSAGE){
        readError();
        socket->flush();
        return;
    }else if (datas.toStdString() == AVATAR_MESSAGE){
        readUser();
        if (clientIsLogged){
            emit okEditAccount();
        }
        clientIsLogged = true;
        onReadyRead();
    }*/

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
        }else if (state == LIST_OF_FILE_RECIVED && datas.toStdString() == SENDING_FILE){
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
        }else if (datas.toStdString() == ERR_MESSAGE){
            if (!readError()){
                return;
            }
        }
    }

    /*if (clientIsLogged){
        if (datas.toStdString() == OK_MESSAGE){
            // capire quale messaggio di ok è stato ricevuto...
        }else if (datas.toStdString() == INSERT_MESSAGE){
            if (!readInsert()){
                return;
            }
        }else if (datas.toStdString() == STYLE_CAHNGED_MESSAGE){
            if (!readStyleChanged()){
                return;
            }
        }else if (datas.toStdString() == DELETE_MESSAGE){
            if (!readDelete()){
                return;
            }
        }else if (datas.toStdString() == LIST_OF_FILE){
            if (!readFileNames()){
                return;
            }
            reciveOkMessage = true;
            #if !UI
                    requestForFile("prova");         TEST: TEXT EDITOR
            #endif
        } else if (datas.toStdString() == LIST_OF_USERS){
            if (!readUsernames()){
                return;
            }
        } else if (datas.toStdString() == REMOVE_USER){
            if (!readRemoveUser()){
                return;
            }
        }else if (datas.toStdString() == SENDING_FILE){
            if (!readFile()){
                return;
            }
        }
        despatchMessage();
        onReadyRead();
    }*/
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
    }

    return true;
}

bool Messanger::despatchMessage(){
    if (!messages.empty()){
        QByteArray message = messages.front();
        messages.pop();
        if (!writeMessage(socket, message)){
            // push ???
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
        QByteArray usernameSize = convertionNumber(username.size());
        QByteArray passwordSize = convertionNumber(password.size());

        message.append(" " + usernameSize + " " + username.toUtf8()  + " " + passwordSize + " " + password.toUtf8());
        qDebug() << "                        " << message;
        qDebug() << ""; // newLine

        if (!writeMessage(socket, message)){
            return false;
        }
    }

    this->crdt->setSiteId(username);    //TODO: da rimuovere...
    this->siteId = username;            //TODO: da rimuovere...

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

    QByteArray usernameSize = convertionNumber(username.size());
    QByteArray passwordSize = convertionNumber(password.size());
    QByteArray imageSize = convertionNumber(avatar.size());

    message.append(" " + usernameSize + " " + username.toUtf8() + " " + passwordSize + " " + password.toUtf8() + " " + imageSize + " " + avatar);

    qDebug() << "                                " << "username: " << username << " password: " << password << " avatarSize: " << avatar.size();
    qDebug() << ""; // newLine

    if (!writeMessage(socket, message)){
        return false;
    }

    this->crdt->setSiteId(username);
    this->siteId = username;

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

    QByteArray usernameSize = convertionNumber(username.size());
    QByteArray newPasswordSize = convertionNumber(newPassword.size());
    QByteArray oldPasswordSize = convertionNumber(oldPassword.size());
    QByteArray imageSize = convertionNumber(avatar.size());

    message.append(" " + usernameSize + " " + username.toUtf8() + " " + newPasswordSize + " " + newPassword.toUtf8() + " "
    + oldPasswordSize + " " + oldPassword.toUtf8() + " " + imageSize + " " + avatar);

    qDebug() << "                                " << message;
    qDebug() << ""; // newLine

    if (!writeMessage(socket, message)){
        return false;
    }

    this->crdt->setSiteId(username);
    this->siteId = username;

    return true;
}

/**
 * This methods recives user's features
 * @return  result of reading from socket
 */
bool Messanger::readUser(){
    qDebug() << "Messanger.cpp - readUser()     ---------- READUSER ----------";
    readSpace(socket);
    int usernameSize = readNumberFromSocket(socket);

    readSpace(socket);
    QByteArray username;

    if (!readChunck(socket, username, usernameSize)){
        return false;
    }

    readSpace(socket);
    int imageSize = readNumberFromSocket(socket);

    qDebug() << imageSize;

    readSpace(socket);
    QByteArray avatar;

    if (!readChunck(socket, avatar, imageSize)){
        return false;
    }

    QImage image1 = QImage::fromData(avatar,"PNG");
    QPixmap pixmap = QPixmap::fromImage(image1);
    User *user1 = new User(username, pixmap);
    user = user1;

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
    std::map<QString, bool> files;

    readSpace(socket);
    int numFiles = readNumberFromSocket(socket);

    qDebug() << "                                numFiles: " << numFiles;

    for (int i = 0; i < numFiles; i++){
        readSpace(socket);
        int fileNameSize = readNumberFromSocket(socket);
        readSpace(socket);
        QByteArray fileName;
        if (!readChunck(socket, fileName, fileNameSize)){
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
        disconnect(c);
        disconnect(d);
        socket->deleteLater();
        delete socket;
        socket = new QTcpSocket();
        if (!connectTo(serverIP, serverPort)){
            qDebug() << "Connesione fallita";
            return false;
        }
        clientIsLogged = false;
        state = UNLOGGED;
        c = connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        d = connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
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
        QByteArray fileNameSize = convertionNumber(fileName.size());

        message.append(" " + fileNameSize + " " + fileName.toUtf8());
        qDebug() << "                                 " << message;
        qDebug() << ""; // newLine

        messages.push(message);

        reciveOkMessage = true;
        if (!writeMessage(socket, message)){
            return false;
        }
        messages.pop();

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
            QByteArray username;
            if (!readChunck(socket, username, usernameSize)){
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
    QByteArray username;
    readSpace(socket);
    if (!readChunck(socket, username, usernameSize)){
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
    }

    emit fileRecive(file);
    return true;
}

/**
 * This method sends the character, that was added
 * @param character
 * @return result of writing on socket
 */
bool Messanger::writeInsert(Character character){
    qDebug() << "Messanger.cpp - writeInsert()     ---------- WRITE INSERT ----------";

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

        if (!writeMessage(socket, message)){
            return false;
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

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    Message message(character, socket->socketDescriptor(), INSERT);

    emit newMessage(message);
    return true;
}

bool Messanger::readStyleChanged(){
    qDebug() << "Messanger.cpp - readStyleChanged()     ---------- READ STYLE CHANGED ----------";

    readSpace(socket);
    int messageSize = readNumberFromSocket(socket);
    readSpace(socket);

    QByteArray characterByteFormat;
    if (!readChunck(socket, characterByteFormat, messageSize)){
        return false;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    Message message(character, socket->socketDescriptor(), STYLE_CHANGED);

    emit newMessage(message);
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

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    Message message(character, socket->socketDescriptor(), DELETE);

    emit newMessage(message);
    return true;
}

void Messanger::onDisconnect(){
    qDebug() << "Messanger.cpp - onDisconnect()     " << socketDescriptor <<" Disconnected";
    qDebug() << ""; // newLine

    QTcpSocket soc;
    soc.setSocketDescriptor(socketDescriptor);
    soc.deleteLater();
    emit errorConnection();
}