#include "Client.h"
#include "../utils/Identifier.h"

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
    return socket->waitForConnected();      /* possibile gestione con un eccezione per il retry */
}

void Client::onReadyRead(){
    QByteArray datas = socket->read(5);
    qDebug() << datas;
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
    }
}

void Client::logIn(QString username, QString password){
    if (!clientIsLogged){
        //writeOnSocket(QString(LOGIN_MESSAGE));
        /* TO-DO: send username and password */
        QByteArray message(LOGIN_MESSAGE);
        QByteArray data;
        QDataStream in(&data,  QIODevice::WriteOnly);
        data.append(" ");
        in << username.size();

        //data.append(" " + username.size());
        data.append(" "+ username + " " + password.size() + " " + password);
        message.append(data);
        qDebug() << message;

        socket->write(message);

        clientIsLogged = true;
    }
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

void Client::requestForFile(QString fileName){
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
    }
}

void Client::insert(QString str, QString siteId, int pos){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(INSERT_MESSAGE);
        QByteArray data;
        QDataStream in(&data,  QIODevice::WriteOnly);
        data.append(" ");
        in << str.size();

        data.append(" " + str + " " + siteId.size() + " " + siteId + " " + pos);
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
        Identifier identifier(pos, siteId.toStdString());
        position.push_back(identifier);
        qDebug() << " pos:" << pos;
        if (i != size.toHex().toInt(&ok,16) - 1 || size.toHex().toInt(&ok,16) != 1){
            socket->read(1);
        }
    }

    Character character(letter[0], 0, siteId.toStdString(), position);
    Message message(character, socket->socketDescriptor(), INSERT);
    incomingMessagesQueue.push(message);
    /* TO-DO: emit signal */

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
        Identifier identifier(pos, siteId.toStdString());
        position.push_back(identifier);
        qDebug() << " pos:" << pos;
        if (i != size.toHex().toInt(&ok,16) - 1 || size.toHex().toInt(&ok,16) != 1){
            socket->read(1);
        }
    }

    Character character(letter[0], 0, siteId.toStdString(), position);
    Message message(character, socket->socketDescriptor(), DELETE);
    incomingMessagesQueue.push(message);
    /* TO-DO: emit signal */

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
    qDebug() << "Disconnected";
    if (socket != nullptr){
        socket->deleteLater();
    }
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
