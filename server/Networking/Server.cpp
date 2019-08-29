#include <QDataStream>
#include <QPixmap>
#include "Server.h"

Server::Server(QObject *parent):QTcpServer(parent){}

bool Server::startServer(quint16 port){
    connect(this,SIGNAL(newConnection()),this,SLOT(connection()));

    if(!this->listen(QHostAddress::Any, port)){
        qDebug() << "Could not start server";
        return false;
    }else{
        qDebug() << "Listening to port " << port << "...";
        return true;
    }
}

void Server::connection(){
    QTcpSocket *soc = this->nextPendingConnection();

    socketsState[soc->socketDescriptor()] = UNLOGGED;
    QMetaObject::Connection *c = new QMetaObject::Connection();

    *c = connect(soc, &QTcpSocket::readyRead, this, [this,c,soc]{
        QByteArray data;
        if (!readChunck(soc, data, 5)){
            writeErrMessage(soc);
            soc->flush();
            return;
        }

        qDebug() << data;

        if (data.toStdString() == LOGIN_MESSAGE){
            if (logIn(soc)){
                sendFileNames(soc);
                socketsState[soc->socketDescriptor()] = LOGGED;
                qDebug() << "socketsSize: " << socketsState.size();
            }else{
                //error in login phase
                qDebug() << "error login";
                writeErrMessage(soc);
                return;
            }
        }else if (data.toStdString() == REGISTRATION_MESSAGE && socketsState[soc->socketDescriptor()] == UNLOGGED){
            if (registration(soc)){
                socketsState[soc->socketDescriptor()] = LOGGED;
            }else{
                writeErrMessage(soc);
                return;
            }
        }else if (data.toStdString() == REQUEST_FILE_MESSAGE && socketsState[soc->socketDescriptor()] == LOGGED) {
            /* disconnect from main thread */
            disconnect(*c);
            delete c;
            socketsState.erase(soc->socketDescriptor());
            qDebug() << "socketsSize: " << socketsState.size();
            
            if (!readFileName(soc->socketDescriptor(), soc)){
                writeErrMessage(soc);
            }
        }else{
            qDebug() << "error message";
            writeErrMessage(soc);
        }
    }, Qt::DirectConnection);
}

bool Server::logIn(QTcpSocket *soc){
    /* read user and password on socket*/
    qDebug() << "-------------LOGIN-------------";

    /* usernameSize */
    soc->read(1);
    int usernameSize = readNumberFromSocket(soc);

    qDebug() << "usernameSize: " <<usernameSize;

    soc->read(1);
    /* username */
    QByteArray username;
    if (!readChunck(soc, username, usernameSize)){
        return false;
    }
    soc->read(1);

    /* passwordSize */
    int passwordSize = readNumberFromSocket(soc);
    soc->read(1);

    qDebug() << "passwordSize: " <<passwordSize;

    QByteArray password;
    if (!readChunck(soc, password, passwordSize)){
        return false;
    }

    qDebug() << "username: " << username << " password: " << password;

    // TODO: richiamo funzione per il login sul db

    return true;
}

bool Server::sendFileNames(QTcpSocket *soc){
    qDebug() << "-------------LIST OF FILE-------------";
    // TODO gestione file dell'utente

    int nFiles = 1;
    QString fileName;
    fileName = "file1";                                     /* file fantoccio: da rimuovere in seguito */
    QByteArray message(LIST_OF_FILE);

    QByteArray numFiles = convertionNumber(nFiles);

    QByteArray fileNameSize = convertionNumber(fileName.size());

    message.append(" " + numFiles + " " + fileNameSize + " " + fileName.toUtf8());
    qDebug() << message;
    soc->write(message);
    if (soc->waitForBytesWritten(TIMEOUT)){
        qDebug() << "LIST OF FILE, scritto";
        return true;
    }else{
        qDebug() << "LIST OF FILE, non scritto";
        return false;
    }
}

bool Server::readFileName(qintptr socketDescriptor, QTcpSocket *soc){
    qDebug() << "-------------REQUEST FOR FILE-------------";
    soc->read(1);                                                       // " "
    int fileNameSize = readNumberFromSocket(soc);
    soc->read(1);                                                       // " "

    QByteArray fileName;
    if (!readChunck(soc, fileName, fileNameSize)){
        writeErrMessage(soc);
        return false;
    }
    qDebug() << fileName;

    std::string key = fileName.toStdString();                           /* file name */
    auto result = threads.find(key);

    if (result != threads.end()){
        /* file already open */
        qDebug() << "thread for file name aready exist " << fileName;
        threads[key]->addSocket(soc);                       /* socket transition to secondary thread */
    }else{
        /* file not yet open */
        qDebug() << "New thread for file name: " << fileName;
        CRDT *crdt = new CRDT();
        Thread *thread = new Thread(this, crdt);                        /* create new thread */
        threads[key] = std::shared_ptr<Thread>(thread);
        thread->addSocket(soc);                            /* socket transition to secondary thread */
        thread->start();
    }

    writeOkMessage(soc);
    return true;
}

bool Server::registration(QTcpSocket *soc){
    qDebug() << "-------------REGISTRATION-------------";
    if (soc == nullptr){
        return false;
    }
    soc->read(1);                                       // " "
    int sizeUsername = readNumberFromSocket(soc);
    soc->read(1);                                       // " "

    //username
    QByteArray username;
    if (!readChunck(soc, username, sizeUsername)){
        writeErrMessage(soc);
    }
    soc->read(1);                                       // " "

    int sizePassword = readNumberFromSocket(soc);
    soc->read(1);                                       // " "

    //password
    QByteArray password;
    if (!readChunck(soc, password, sizePassword)){
        writeErrMessage(soc);
    }
    soc->read(1);                                       // " "

    QDataStream in(soc);
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