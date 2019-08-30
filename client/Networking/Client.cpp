#include "Client.h"
#include "../utils/Identifier.h"
#include "common/commonFunctions.h"
#include <QFile>
#include <QPixmap>
#include <iostream>

Client::Client(QObject *parent):QObject (parent){
    this->socket = new QTcpSocket(this);
    reciveOkMessage = false;
    clientIsLogged = false;

    /* define connection */
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
}

bool Client::connectTo(QString host){
    socket->connectToHost(host, 1234);

    //return socket->waitForConnected();      /* possibile gestione con un eccezione per il retry */
    if (!socket->waitForConnected(30)){
        emit errorConnection();
        return false;
    }

    socketDescriptor = socket->socketDescriptor();
    qDebug() << socket->socketDescriptor() <<" connected";
    return true;
}

void Client::onReadyRead(){
    if (socket->bytesAvailable() == 0){
        return;
    }

    QByteArray datas;
    if (!readChunck(socket, datas, 5)){
        return;
    }
    qDebug() << "msg received: " << datas;

    if ((datas.toStdString() == OK_MESSAGE || datas.toStdString() == LIST_OF_FILE) && !clientIsLogged){
        clientIsLogged = true;
    }else if (!clientIsLogged && datas.toStdString() == ERR_MESSAGE){
        emit loginFailed();
        return;
    }

    if (clientIsLogged){
        if (datas.toStdString() == OK_MESSAGE){
            if (!messages.empty()){
                QByteArray message = messages.front();
                if (message == LOGOUT_MESSAGE){
                    socket->deleteLater();
                    return;
                }
                messages.pop();
                if (!writeMessage(socket, message)){
                    // push ???
                    return;
                }
                reciveOkMessage = false;
            }else{
                reciveOkMessage = true;
            }
        }else if (datas.toStdString() == INSERT_MESSAGE){
            if (readInsert()){
                reciveOkMessage = true;
                onReadyRead();
            }
        }else if (datas.toStdString() == DELETE_MESSAGE){
            if (readDelete()){
                reciveOkMessage = true;
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
            requestForFile("prova");        /* TEST: TEXT EDITOR */
        }
    }
}

bool Client::logIn(QString username, QString password){
    qDebug() << "-------------LOGIN-------------";
    //TODO: Connessione al server, verifica di credenziali...
    if( username=="test" && password=="test" ){                 //only for testing...
        return true;
    }

    if (!clientIsLogged) {
        QByteArray message(LOGIN_MESSAGE);
        QByteArray usernameSize = convertionNumber(username.size());
        QByteArray passwordSize = convertionNumber(password.size());

        message.append(" " + usernameSize + " " + username.toUtf8()  + " " + passwordSize + " " + password.toUtf8());
        qDebug() << message;

        if (!writeMessage(socket, message)){
            return false;
        }
    }
    return true;
}

bool Client::registration(QString username, QString password, QString pathAvatar){
    qDebug() << "-------------REGISTRATION-------------";
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

    qDebug() << username << " " << password << " avatarSize: " << pix.toImage().sizeInBytes();
    return true;
}

bool Client::readFileNames(){
    qDebug() << "-------------READ FILE NAMES-------------";
    QStringList fileList;

    readSpace(socket);
    int numFiles = readNumberFromSocket(socket);

    qDebug () << "numFiles" << numFiles;

    for (int i = 0; i < numFiles; i++){
        readSpace(socket);
        int fileNameSize = readNumberFromSocket(socket);
        readSpace(socket);
        QByteArray fileName;
        if (!readChunck(socket, fileName, fileNameSize)){
            return false;
        }
        fileList.append(fileName);

        qDebug () << fileName;
    }

    emit fileNames(fileList);

    return true;
}

void Client::logOut(){
    if (clientIsLogged){
        QByteArray message(LOGOUT_MESSAGE);
        messages.push(message);
        if (reciveOkMessage){
            messages.pop();
            socket->deleteLater();
        }
    }
}

bool Client::requestForFile(QString fileName){
    qDebug() << "-------------REQUEST FOR FILE-------------";
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(REQUEST_FILE_MESSAGE);
        QByteArray fileNameSize = convertionNumber(fileName.size());

        message.append(" " + fileNameSize + " " + fileName.toUtf8());
        qDebug() << message;

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

bool Client::insert(QString str, QString siteId, Pos pos){
    qDebug() << "-------------WRITE INSERT-------------";
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(INSERT_MESSAGE);
        QByteArray strSize = convertionNumber(str.size());
        QByteArray siteIdSize = convertionNumber(siteId.size());
        QByteArray posCh = convertionNumber(pos.getCh());
        QByteArray posLine = convertionNumber(pos.getLine());

        message.append(" " + strSize + " " + str.toUtf8() + " " + siteIdSize + " " + siteId.toUtf8() + " " + posCh + " " + posLine);
        qDebug() << message;
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

bool Client::deleteChar(QString str, QString siteId, std::vector<Identifier> pos){
    qDebug() << "-------------WRITE DELETE-------------";
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
        qDebug() << message;

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

bool Client::readInsert(){
    qDebug() << "-------------READ INSERT-------------";
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
    qDebug() << "ch: "<<letter << " siteId: "<< siteId;

    for (int i = 0; i < posSize; i++){
        int pos = readNumberFromSocket(socket);
        Identifier identifier(pos, siteId);
        position.push_back(identifier);
        qDebug() << pos;
        if (i != posSize - 1 || posSize != 1){
            readSpace(socket);
        }
    }

    Character character(letter[0], 0, siteId, position);
    Message message(character, socket->socketDescriptor(), INSERT);
    incomingInsertMessagesQueue.push(message);

    emit newMessage(message);
    return true;
}

bool Client::readDelete(){
    qDebug() << "-------------READ DELETE-------------";
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

    qDebug() << "ch: "<<letter << " siteId: "<< siteId;

    for (int i = 0; i < size; i++){
        int pos = readNumberFromSocket(socket);
        Identifier identifier(pos, siteId);
        position.push_back(identifier);
        qDebug() << " pos:" << pos;
        if (i != size - 1 || size != 1){
            readSpace(socket);
        }
    }

    Character character(letter[0], 0, siteId, position);
    Message message(character, socket->socketDescriptor(), DELETE);
    incomingDeleteMessagesQueue.push(message);

    emit newMessage(message);
    return true;
}

void Client::onDisconnect(){
    qDebug() << socketDescriptor <<" Disconnected";

    QTcpSocket soc;
    soc.setSocketDescriptor(socketDescriptor);
    soc.deleteLater();
    emit errorConnection();
}

Message Client::getMessage() {

    std::cout << "incomingInsertMessagesQueue size: " << incomingInsertMessagesQueue.size() << std::endl;

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