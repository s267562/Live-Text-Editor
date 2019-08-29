#include "Client.h"
#include "../utils/Identifier.h"
#include "common/commonFunctions.h"

#include <QFile>
#include <QPixmap>

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
    QByteArray datas = socket->read(5);
    qDebug() << datas;

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
                socket->write(message);
                reciveOkMessage = false;
            }else{
                reciveOkMessage = true;
            }
        }else if (datas.toStdString() == INSERT_MESSAGE){
            if (readInsert()){
                reciveOkMessage = true;
            }
        }else if (datas.toStdString() == DELETE_MESSAGE){
            if (readDelete()){
                reciveOkMessage = true;
            }
        }else if (datas.toStdString() == LIST_OF_FILE){
            readFileNames();
            reciveOkMessage = true;
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

        socket->write(message);
    }
     return false;
}

void Client::registration(QString username, QString password, QString pathAvatar){
    qDebug() << "-------------REGISTRATION-------------";
    QPixmap pix;
    pix.load(pathAvatar);
    QByteArray image = QByteArray::fromRawData((const char*)pix.toImage().bits(), pix.toImage().sizeInBytes());

    QDataStream out(socket);
    QByteArray message(REGISTRATION_MESSAGE);

    QByteArray usernameSize = convertionNumber(username.size());
    QByteArray passwordSize = convertionNumber(password.size());
    message.append(" " + usernameSize + " " + username.toUtf8() + " " + passwordSize + " " + password.toUtf8() + " ");
    socket->write(message);

    //avatar
    out << pix.toImage().sizeInBytes();
    socket->write(" ");
    socket->write(image);

    qDebug() << username << " " << password << " avatarSize: " << pix.toImage().sizeInBytes();
}

bool Client::readFileNames(){
    qDebug() << "-------------READ FILE NAMES-------------";
    QStringList fileList;

    socket->read(1);
    int numFiles = readNumberFromSocket(socket);

    qDebug () << "numFiles" << numFiles;

    for (int i = 0; i < numFiles; i++){
        socket->read(1);
        int fileNameSize = readNumberFromSocket(socket);
        socket->read(1);
        QString fileName = socket->read(fileNameSize);
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
            this->socket->write(message);
            messages.pop();
        }
        return true;
    }
    return false;
}

void Client::insert(QString str, QString siteId, Pos pos){
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
            this->socket->write(message);
            messages.pop();
        }
    }
}

bool Client::readInsert(){
    qDebug() << "-------------READ INSERT-------------";
    socket->read(1);
    int sizeString = readNumberFromSocket(socket);
    socket->read(1);                                // " "

    QByteArray letter;
    if (!readChunck(socket, letter, sizeString)){
        return false;
    }
    socket->read(1);                                // " "

    //siteID
    int siteIdSize = readNumberFromSocket(socket);
    socket->read(1);                                // " "
    QByteArray siteId;
    if (!readChunck(socket, siteId, siteIdSize)){
        return false;
    }
    socket->read(1);                                // " "
    int posSize = readNumberFromSocket(socket);

    socket->read(1);                                // " "
    std::vector<Identifier> position;
    qDebug() << "ch: "<<letter << " siteId: "<< siteId;

    for (int i = 0; i < posSize; i++){
        int pos = readNumberFromSocket(socket);
        Identifier identifier(pos, siteId);
        position.push_back(identifier);
        qDebug() << pos;
        if (i != posSize - 1 || posSize != 1){
            socket->read(1);                        // " "
        }
    }

    Character character(letter[0], 0, siteId, position);
    Message message(character, socket->socketDescriptor(), INSERT);
    incomingInsertMessagesQueue.push(message);

    //emit newMessage();
    return true;
}

bool Client::readDelete(){
    qDebug() << "-------------DEL-------------";
    bool ok;
    QDataStream in(socket);
    int sizeString;
    in >> sizeString;
    socket->read(1);
    QByteArray letter = socket->read(sizeString);
    socket->read(1);

    //siteID
    QByteArray sizeSiteId = socket->read(1);
    socket->read(1);
    QByteArray siteId = socket->read(sizeSiteId.toHex().toInt(&ok,16));
    qDebug()<< siteId << " size" << sizeSiteId.toHex().toInt(&ok,16);
    socket->read(1);

    QByteArray size = socket->read(1);

    qDebug() << " size pos:" << size.toHex().toInt(&ok,16);
    socket->read(1);
    std::vector<Identifier> position;
    qDebug() << letter;

    for (int i = 0; i < size.toHex().toInt(&ok,16); i++){
        int pos = socket->read(1).toHex().toInt(&ok,16);
        Identifier identifier(pos, siteId);
        position.push_back(identifier);
        qDebug() << " pos:" << pos;
        if (i != size.toHex().toInt(&ok,16) - 1 || size.toHex().toInt(&ok,16) != 1){
            socket->read(1);
        }
    }

    Character character(letter[0], 0, siteId, position);
    Message message(character, socket->socketDescriptor(), DELETE);
    incomingDeleteMessagesQueue.push(message);

    emit newMessage();
    return true;
}

void Client::deleteChar(QString str, QString siteId, std::vector<Identifier> pos){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(DELETE_MESSAGE);
        QByteArray data;
        data.append(" " + str + " "  + siteId.size() + " " + siteId + " " + pos.size() + " ");
        QByteArray position;

        for (int i = 0; i < pos.size(); i++){
            position.append(pos[i].getDigit());
            if (i != pos.size() - 1 || pos.size() != 1){
                position.append(" ");
            }
        }

        data.append(position);
        //message.append(data.size());
        message.append(data);
        qDebug() << message;

        messages.push(message);
        if (reciveOkMessage){
            reciveOkMessage = false;
            this->socket->write(message);
            messages.pop();
        }
    }
}

bool Client::writeOnSocket(QString str){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        this->socket->write(str.toUtf8());
        return this->socket->waitForBytesWritten();
    }else{
        return false;
    }
}

void Client::onDisconnect(){
    qDebug() << socketDescriptor <<" Disconnected";

    QTcpSocket soc;
    soc.setSocketDescriptor(socketDescriptor);
    soc.deleteLater();

    //emit errorConnection();
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