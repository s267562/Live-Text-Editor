#include "Client.h"
#include "../utils/Identifier.h"
#include "common/commonFunctions.h"
#include <QFile>
#include <QPixmap>
#include <iostream>

Client::Client(QObject *parent) : QObject(parent) {
    this->socket = new QTcpSocket(this);
    reciveOkMessage = false;

    /* define connection */
    c = connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    d = connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}

void Client::setCRDT(CRDT *crdt) {
    this->crdt = crdt;
}

bool Client::connectTo(QString host){
    socket->connectToHost(host, 1234);

    //return socket->waitForConnected();      /* possibile gestione con un eccezione per il retry */
    if (!socket->waitForConnected(30)){
        emit errorConnection();
        return false;
    }

    socketDescriptor = socket->socketDescriptor();

    qDebug() << "Client.cpp - connectTo()     " << socket->socketDescriptor() << " connected";
    qDebug() << ""; // newLine
    return true;
}

int i = 0;

void Client::onReadyRead(){
    if (socket->bytesAvailable() == 0){
        return;
    }

    QByteArray datas;
    if (!readChunck(socket, datas, 5)){
        return;
    }

    qDebug() << "Client.cpp - onReadyRead()     msg received:" << datas;
    qDebug() << ""; // newLine

    if ((datas.toStdString() == OK_MESSAGE || datas.toStdString() == LIST_OF_FILE) && !clientIsLogged){
        clientIsLogged = true;
        reciveOkMessage = true;
    }else if (!clientIsLogged && datas.toStdString() == ERR_MESSAGE){
        emit loginFailed();
        return;
    }

    if (clientIsLogged){
        if (datas.toStdString() == OK_MESSAGE){
            if (!messages.empty()){
                QByteArray message = messages.front();
                messages.pop();
                if (!writeMessage(socket, message)){
                    // push ???
                    return;
                }
                /*if (message == LOGOUT_MESSAGE){
                    clientIsLogged = false;
                    logIn("Ciao","Ciao");
                }*/
                reciveOkMessage = false;
            }else{
                reciveOkMessage = true;
            }
        }else if (datas.toStdString() == INSERT_MESSAGE){
            if (readInsert()){
                if (!messages.empty()){
                    QByteArray message = messages.front();
                    messages.pop();
                    if (!writeMessage(socket, message)){
                        // push ???
                        return;
                    }
                    reciveOkMessage = false;
                }else{
                    reciveOkMessage = true;
                }
                //reciveOkMessage = true;
                onReadyRead();
            }
        }else if (datas.toStdString() == DELETE_MESSAGE){
            if (readDelete()){
                if (!messages.empty()){
                    QByteArray message = messages.front();
                    messages.pop();
                    if (!writeMessage(socket, message)){
                        // push ???
                        return;
                    }
                    reciveOkMessage = false;
                }else{
                    reciveOkMessage = true;
                }
                onReadyRead();
            }
        }else if (datas.toStdString() == LIST_OF_FILE){
            readFileNames();
            reciveOkMessage = true;
#if !MAINWINDOW
            requestForFile("prova");        /* TEST: TEXT EDITOR */
#endif
        }else if (datas.toStdString() == RESET_MESSAGE) {
            if (readReset()){
                reciveOkMessage = true;
                onReadyRead();
            }
        }
    }
}

bool Client::logIn(QString username, QString password) {
    qDebug() << "Client.cpp - logIn()     ---------- LOGIN ----------";
    //TODO: Connessione al server, verifica di credenziali...
    if( username=="test" && password=="test" ){                 //only for testing...
        return true;
    }

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

    this->crdt->setSiteId(username);
    this->siteId = username;

    return true;
}

bool Client::registration(QString username, QString password, QString pathAvatar){
    qDebug() << "Client.cpp - registration()     ---------- REGISTRATION ----------";
    QPixmap pix;
    pix.load(pathAvatar);
    QByteArray image = QByteArray::fromRawData((const char*)pix.toImage().bits(), pix.toImage().sizeInBytes());

    QDataStream out(socket);
    QByteArray message(REGISTRATION_MESSAGE);

    QByteArray usernameSize = convertionNumber(username.size());
    QByteArray passwordSize = convertionNumber(password.size());
    message.append(" " + usernameSize + " " + username.toUtf8() + " " + passwordSize + " " + password.toUtf8() + " ");
    if (!writeMessage(socket, message)){
        return false;
    }

    //avatar
    out << pix.toImage().sizeInBytes();
    socket->write(" ");

    if (!writeMessage(socket, image)){
        return false;
    }

    qDebug() << "                                " << "username: " << username << " password: " << password << " avatarSize: " << pix.toImage().sizeInBytes();
    qDebug() << ""; // newLine

    this->crdt->setSiteId(username);
    this->siteId = username;

    return true;
}

bool Client::readFileNames(){
    qDebug() << "Client.cpp - readFileName()     ---------- READ FILE NAME ----------";
    QStringList fileList;

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
        fileList.append(fileName);

        qDebug() << "                               " << i + 1 << "." << fileName;
    }

    qDebug() << ""; // newLine

    emit fileNames(fileList);

    return true;
}

void Client::logOut(){
    if (clientIsLogged){
        /*qDebug() << "Logout";
        QByteArray message(LOGOUT_MESSAGE);
        messages.push(message);
        //socket->write(message);
        if (reciveOkMessage){
            //socket->write(message);
            messages.pop();
        }*/
        disconnect(c);
        disconnect(d);
        socket->deleteLater();
        delete socket;
        socket = new QTcpSocket();
        if (!connectTo("127.0.0.1")){
            qDebug() << "Connesione fallita";
            return;
        }
        clientIsLogged = false;
        c = connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        d = connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
        emit logout();
    }
}

bool Client::requestForFile(QString fileName){
    qDebug() << "Client.cpp - requestForFile()     ---------- REQUEST FOR FILE ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(REQUEST_FILE_MESSAGE);
        QByteArray fileNameSize = convertionNumber(fileName.size());

        message.append(" " + fileNameSize + " " + fileName.toUtf8());
        qDebug() << "                                 " << message;
        qDebug() << ""; // newLine

        messages.push(message);
        if (reciveOkMessage){
            reciveOkMessage = false;
            if (!writeMessage(socket, message)){
                return false;
            }
            messages.pop();
        }
        return true;
    }
    return false;
}

bool Client::insert(QString str, Pos pos){
    qDebug() << "Client.cpp - insert()     ---------- WRITE INSERT ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(INSERT_MESSAGE);
        QByteArray strSize = convertionNumber(str.size());
        QByteArray siteIdSize = convertionNumber(siteId.size());
        QByteArray posCh = convertionNumber(pos.getCh());
        QByteArray posLine = convertionNumber(pos.getLine());

        message.append(" " + strSize + " " + str.toUtf8() + " " + siteIdSize + " " + siteId.toUtf8() + " " + posCh + " " + posLine);
        qDebug() << "                         " << message;
        qDebug() << ""; // newLine
        messages.push(message);
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

bool Client::deleteChar(QString str, std::vector<Identifier> pos){
    qDebug() << "Client.cpp - insert()     ---------- WRITE DELETE ----------";

    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(DELETE_MESSAGE);
        QByteArray siteIdSize = convertionNumber(siteId.size());
        QByteArray posSize = convertionNumber(pos.size());
        message.append(" " + str.toUtf8() + " "  + siteIdSize + " " + siteId.toUtf8() + " " + posSize + " ");
        QByteArray position;

        for (int i = 0; i < pos.size(); i++){
            position.append(convertionNumber(pos[i].getDigit()));
            if (i != pos.size() - 1 || pos.size() != 1){
                position.append(" ");
            }
        }

        message.append(position);
        qDebug() << "                         " << message;
        qDebug() << ""; // newLine

        messages.push(message);
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

void Client::resetModel(QString siteId) {
    qDebug() << "Client.cpp - resetModel()     ---------- WRITE RESET_MODEL ----------";
    qDebug() << ""; // newLine

    // write Reset_Model
    //TODO to implement
}


bool Client::readReset() {
    qDebug() << "Client.cpp - readReset()     ---------- READ RESET_MODEL ----------";
    qDebug() << ""; // newLine
    readSpace(socket);

    //siteID
    int siteIdSize = readNumberFromSocket(socket);
    readSpace(socket);
    QByteArray siteId;
    if (!readChunck(socket, siteId, siteIdSize)){
        return false;
    }

    // TODO c'è altro da fare?

    emit reset(siteId);
    return true;
}


bool Client::readInsert(){
    qDebug() << "Client.cpp - readInsert()     ---------- READ INSERT ----------";

    readSpace(socket);
    int sizeString = readNumberFromSocket(socket);
    readSpace(socket);

    QByteArray letter;
    if (!readChunck(socket, letter, sizeString)){
        return false;
    }
    readSpace(socket);

    //siteID
    int siteIdSize = readNumberFromSocket(socket);
    readSpace(socket);
    QByteArray siteId;
    if (!readChunck(socket, siteId, siteIdSize)){
        return false;
    }
    readSpace(socket);
    int posSize = readNumberFromSocket(socket);

    readSpace(socket);
    std::vector<Identifier> position;
    qDebug() << "                              ch: "<<letter << " siteId: "<< siteId;

    for (int i = 0; i < posSize; i++){
        int pos = readNumberFromSocket(socket);
        Identifier identifier(pos, siteId);
        position.push_back(identifier);
        qDebug() << "                             " << pos;
        if (i != posSize - 1 || posSize != 1){
            readSpace(socket);
        }
    }
    qDebug() << ""; // newLine

    Character character(letter[0], 0, siteId, position);
    Message message(character, socket->socketDescriptor(), INSERT);
    incomingInsertMessagesQueue.push(message);

    emit newMessage(message);
    return true;
}

bool Client::readDelete(){
    qDebug() << "Client.cpp - readDelete()     ---------- READ DELETE ----------";

    readSpace(socket);

    QByteArray letter;
    if (!readChunck(socket, letter, 1)){
        return false;
    }
    readSpace(socket);

    //siteID
    int siteIdSize = readNumberFromSocket(socket);
    readSpace(socket);
    QByteArray siteId;
    if (!readChunck(socket, siteId, siteIdSize)){
        return false;
    }
    readSpace(socket);

    int size = readNumberFromSocket(socket);

    readSpace(socket);
    std::vector<Identifier> position;

    qDebug() << "                              ch: "<<letter << " siteId: "<< siteId;

    for (int i = 0; i < size; i++){
        int pos = readNumberFromSocket(socket);
        Identifier identifier(pos, siteId);
        position.push_back(identifier);
        qDebug() << "                              pos:" << pos;
        if (i != size - 1 || size != 1){
            readSpace(socket);
        }
    }
    qDebug() << ""; // newLine

    Character character(letter[0], 0, siteId, position);
    Message message(character, socket->socketDescriptor(), DELETE);
    incomingDeleteMessagesQueue.push(message);

    emit newMessage(message);
    return true;
}



void Client::onDisconnect(){
    qDebug() << "Client.cpp - onDisconnect()     " << socketDescriptor <<" Disconnected";
    qDebug() << ""; // newLine

    QTcpSocket soc;
    soc.setSocketDescriptor(socketDescriptor);
    soc.deleteLater();
    emit errorConnection();
}

Message Client::getMessage() {

    // give priority to insert messages.
    if(!incomingInsertMessagesQueue.empty()) {
        Message message = incomingInsertMessagesQueue.front();
        incomingInsertMessagesQueue.pop();
        return message;
    } else {
        Message message = incomingDeleteMessagesQueue.front();
        incomingDeleteMessagesQueue.pop();
        return message;
    }
}