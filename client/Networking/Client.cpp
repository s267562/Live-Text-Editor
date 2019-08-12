#include "Client.h"
#include "../utils/Identifier.h"

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
    QByteArray datas = socket->readAll();
    qDebug() << datas;
    if (datas.toStdString() == OK_MESSAGE){
        if (!messages.empty()){
            QByteArray message = messages.front();
            messages.pop();
            socket->write(message);
            reciveOkMessage = false;
        }else{
            reciveOkMessage = true;
        }
    }
}


void Client::logIn(QString username, QString password){
    if (!clientIsLogged){
        writeOnSocket(QString(LOGIN_MESSAGE));
        /* TO-DO: send username and password */
    }
}

void Client::requestForFile(QString fileName){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(REQUEST_FILE_MESSAGE);
        QByteArray data;
        data.append(fileName);
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

void Client::insert(QString str, std::vector<Identifier> pos){
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
}

void Client::deleteChar(QString str, int pos){
    if (this->socket->state() == QTcpSocket::ConnectedState){
        QByteArray message(DELETE_MESSAGE);
        QByteArray data;
        data.append(" " + str + " " + pos);
        message.append(data.size());
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
    socket->deleteLater();
}
