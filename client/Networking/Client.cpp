#include "Client.h"
#include "../utils/Identifier.h"

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
            readInsert();
        }else if (datas.toStdString() == DELETE_MESSAGE){
            readDelete();
        }else if (datas.toStdString() == LIST_OF_FILE){
            readFileNames();
            reciveOkMessage = true;
        }
    }
}

bool Client::logIn(QString username, QString password){
    //TODO: Connessione al server, verifica di credenziali...
    if( username=="test" && password=="test" ){
        return true;
    }


    if (!clientIsLogged) {
        QByteArray message(LOGIN_MESSAGE);
        QByteArray data;
        QDataStream in(&data, QIODevice::WriteOnly);
        data.append(" ");
        in << username.size();

        //data.append(" " + username.size());
        data.append(" " + username + " " + password.size() + " " + password);
        message.append(data);
        qDebug() << message;

        socket->write(message);
    }
     return false;
}

bool Client::readFileNames(){
    qDebug() << "-------------READ FILE NAMES-------------";
    QStringList fileList;
    socket->read(1);
    QDataStream inNumFiles(socket);
    int numFiles;
    inNumFiles >> numFiles;
    qDebug () << "numFiles" << numFiles;
    socket->read(1);

    QDataStream inFileNameSize(socket);
    int fileNameSize;
    inFileNameSize >> fileNameSize;

    qDebug () << "filesSize" << fileNameSize;

    socket->read(1);
    QString fileName = socket->read(fileNameSize);
    fileList.append(fileName);

    emit fileNames(fileList);

    qDebug () << "fileName" << fileName;

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
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(REQUEST_FILE_MESSAGE);
        QByteArray data;
        data.append(fileName);
        //
        message.append(data);
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
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(INSERT_MESSAGE);
        QByteArray data;
        QDataStream in(&data,  QIODevice::WriteOnly);
        data.append(" ");
        in << str.size();

        data.append(" " + str + " " + siteId.size() + " " + siteId + " " + pos.getCh() + " " + pos.getLine());
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

bool Client::readInsert(){
    qDebug() << "-------------INS-------------";
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
    Message message(character, socket->socketDescriptor(), INSERT);
    incomingInsertMessagesQueue.push(message);

    emit newMessage();
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
/*void Client::insert(QString str, std::vector<Identifier> pos){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(INSERT_MESSAGE);
        QByteArray data;
        data.append(" " + str + " " + pos.size() + " ");
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

void Client::insert(QString str, std::vector<int> pos){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(INSERT_MESSAGE);
        QByteArray data;
        data.append(" " + str + " " + pos.size() + " ");
        QByteArray position;

        for (int i = 0; i < pos.size(); i++){
            position.append(pos[i]);
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
}*/

void Client::deleteChar(QString str, QString siteId, std::vector<int> pos){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(DELETE_MESSAGE);
        QByteArray data;
        data.append(" " + str + " "  + siteId.size() + " " + siteId + " " + pos.size() + " ");
        QByteArray position;

        for (int i = 0; i < pos.size(); i++){
            position.append(pos[i]);
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

/*void Client::deleteChar(QString str, int pos){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(DELETE_MESSAGE);
        QByteArray data;
        data.append(" " + str + " " + "\x01" + " "+ pos);
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
}*/

bool Client::writeOnSocket(QString str){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        this->socket->write(str.toUtf8());
        return this->socket->waitForBytesWritten();
    }else{
        return false;
    }
}

bool Client::writeOnSocket(std::string str){
    QByteArray data(str.c_str()/*str.length()*/);

    if (this->socket->state() == QTcpSocket::ConnectedState){
        this->socket->write(data);
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

void Client::registration(QString username, QString password, QString pathAvatar){
    QPixmap pix;
    pix.load(pathAvatar);
    QByteArray image = QByteArray::fromRawData((const char*)pix.toImage().bits(), pix.toImage().sizeInBytes());

    QByteArray uSize, pSize, aSize;
    uSize.setNum(username.size());
    pSize.setNum(password.size());
    aSize.setNum(pix.toImage().sizeInBytes());

    //socket->write(REGISTRATION_MESSAGE);
    QByteArray uname;
    uname.append(" " + uSize + " " + username);
    QByteArray pword;
    pword.append(" " + pSize + " " + password + " ");
    QByteArray img;
    img.append(aSize + " " + image);
    /*QByteArray message;
    message.append(REGISTRATION_MESSAGE).append(uname).append(pword).append(img);*/

    QDataStream out(socket);
    socket->write(REGISTRATION_MESSAGE);

    //username
    socket->write(" ");
    out << username.size();
    socket->write(" ");
    socket->write(username.toLocal8Bit());
    socket->write(" ");

    //password
    out << password.size();
    socket->write(" ");
    socket->write(password.toLocal8Bit());
    socket->write(" ");

    //avatar
    out << pix.toImage().sizeInBytes();
    socket->write(" ");
    socket->write(image);


    qDebug() << username << " " << password << " " << aSize;
    qDebug() << sizeof(pix.toImage().sizeInBytes());
    qDebug() << sizeof(username.size());
    //socket->write(message);
    //qDebug() << image;
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