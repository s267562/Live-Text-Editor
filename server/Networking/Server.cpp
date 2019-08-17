#include <QDataStream>
#include <QPixmap>
#include "Server.h"

Server::Server(QObject *parent):QTcpServer(parent){}

void Server::startServer(quint16 port){
    connect(this,SIGNAL(newConnection()),this,SLOT(connection()));

    if(!this->listen(QHostAddress::Any, port)){
        qDebug() << "Could not start server";
    }
    else{
        qDebug() << "Listening to port " << port << "...";
    }
}

void Server::connection(){
    QTcpSocket *soc = this->nextPendingConnection();

    QMetaObject::Connection *c = new QMetaObject::Connection();

    *c = connect(soc, &QTcpSocket::readyRead, this, [this,c,soc]{


        QByteArray data;
        if (!readChunck(soc, data, 5)){
            writeErrMessage(soc);
        }

        qDebug() << data;

        if (data.toStdString() == LOGIN_MESSAGE){
            qDebug() << "ok, login";
            if (logIn(soc)){
                writeOkMessage(soc);

                qintptr socketDescriptor = soc->socketDescriptor();

                if (!readChunck(soc, data, 5)){
                    qDebug() << data;
                    writeErrMessage(soc);
                }

                if (data.toStdString() == REQUEST_FILE_MESSAGE){
                    /* disconnect from main thread */
                    disconnect(*c);
                    delete c;

                    readFileName(socketDescriptor, soc);
                    qDebug() << "ok, file";
                }else{
                    //error in file request phase
                    qDebug() << "err, file";
                    qDebug() << data;
                    writeErrMessage(soc);
                }
            }else{
                //error in login phase
                qDebug() << "error login";
                writeErrMessage(soc);
            }
        }else if (data.toStdString() == REGISTRATION_MESSAGE){
            registration(soc);
        }else{
            qDebug() << "error message";
            writeErrMessage(soc);
        }
    }, Qt::DirectConnection);
}

bool Server::logIn(QTcpSocket *soc){
    /* read user and password on socket*/
    bool ok;
    QByteArray data;
    QDataStream in(soc);
    data.append(" ");
    /*in << str.size();
    soc->read(1);*/
    int usernameSize; /*= soc->read(1).toHex().toInt(&ok,16);*/
    in >> usernameSize;

    qDebug() << "usernameSize: " <<usernameSize;

    soc->read(1);
    QByteArray username;
    if (!readChunck(soc, username, usernameSize)){
        return false;
    }
    soc->read(1);

    int passwordSize = soc->read(1).toHex().toInt(&ok,16);
    soc->read(1);

    qDebug() << "passwordSize: " <<usernameSize;

    QByteArray password;
    if (!readChunck(soc, password, passwordSize)){
        return false;
    }

    qDebug() << "username: " << username << " password: " << password;

    
    return true;
}

bool Server::readFileName(qintptr socketDescriptor, QTcpSocket *soc){
    soc->waitForReadyRead(30);
    QByteArray data = soc->readAll();

    std::string key = data.toStdString();                           /* file name */
    qDebug() << data;
    auto result = threads.find(key);

    if (result != threads.end()){
        /* file already open */
        qDebug() << "thread for file name aready exist " << data;
        threads[key]->addSocket(socketDescriptor);                  /* socket transition to secondary thread */
    }else{
        /* file not yet open */
        qDebug() << "New thread for file name: " << data;
        Thread *thread = new Thread(this);     /* create new thread */
        threads[key] = std::shared_ptr<Thread>(thread);
        thread->addSocket(socketDescriptor);                        /* socket transition to secondary thread */
        thread->start();
    }

    writeOkMessage(soc);
    return true;
}

bool Server::registration(QTcpSocket *soc){
    if (soc == nullptr){
        return false;
    }

    QDataStream in(soc);

    soc->read(1);               // " "
    int sizeUsername;
    in >> sizeUsername;
    soc->read(1);               // " "

    //username
    QByteArray username;
    if (!readChunck(soc, username, sizeUsername)){
        writeErrMessage(soc);
    }
    soc->read(1);               // " "

    int sizePassword;
    in >> sizePassword;
    soc->read(1);// " "

    //password
    QByteArray password;
    if (!readChunck(soc, password, sizePassword)){
        writeErrMessage(soc);
    }
    soc->read(1);               // " "

    qsizetype sizeAvatar;
    in >> sizeAvatar;
    soc->read(1);

    qDebug() << username << " " << sizeUsername;
    qDebug() << password << " " << sizePassword;
    qDebug() << "avatar size" << sizeAvatar;

    //avatar
    QByteArray avatarDef;

    if (readChunck(soc, avatarDef, sizeAvatar)){
        writeOkMessage(soc);
    }else{
        writeErrMessage(soc);
    }

    qDebug() << "avatar size" << sizeAvatar << " size read" << avatarDef.size();

    return true;
}