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


        QByteArray data = soc->read(5);

        if (data.toStdString() == LOGIN_MESSAGE){
            qDebug() << "ok, login";
            if (logIn()){
                writeOkMessage(soc);

                qintptr socketDescriptor = soc->socketDescriptor();
                soc->waitForReadyRead(3000);
                data = soc->read(5);
                if (data.toStdString() == REQUEST_FILE_MESSAGE){
                    /* disconnect from main thread */
                    disconnect(*c);
                    delete c;

                    readFileName(socketDescriptor, soc);
                    qDebug() << "ok, file";
                }else{
                    //error in file request phase
                    qDebug() << "err, file";
                    writeErrMessage(soc);
                }
            }else{
                //error in login phase
                qDebug() << "error login";
                writeErrMessage(soc);
            }
        }else {
            qDebug() << "error message";
            writeErrMessage(soc);
        }
    }, Qt::DirectConnection);
}

bool Server::logIn(){
    /* read user and password on socket*/
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
}

bool Server::writeOkMessage(QTcpSocket *soc){
    if (soc == nullptr){
        return false;
    }

    soc->write(OK_MESSAGE);
    if (soc->waitForBytesWritten(30)){
        qDebug() << "Ok, scritto";
        return true;
    }else{
        qDebug() << "Ok, non scritto";
        return false;
    }
}

bool Server::writeErrMessage(QTcpSocket *soc){
    if (soc == nullptr){
        return false;
    }

    soc->write(ERR_MESSAGE);
    if (soc->waitForBytesWritten(30)){
        qDebug() << "Err, scritto";
        return true;
    }else{
        qDebug() << "Err, non scritto";
        return false;
    }
}
