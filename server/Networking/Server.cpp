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
    socket = this->nextPendingConnection();

    QMetaObject::Connection *c = new QMetaObject::Connection();

    *c = connect(socket, &QTcpSocket::readyRead, this, [this,c]{
        /* diconnect from main thread */
        disconnect(*c);
        delete c;

        QByteArray data = socket->read(5);

        if (data.toStdString() == LOGIN_MESSAGE){
            qDebug() << "ok, login";
            if (logIn()){
                socket->write(OK_MESSAGE);
            }else{
                //error in login phase
                socket->write(ERR_MESSAGE);
            }
        }else{
            qDebug() << "err, login";
            socket->write(ERR_MESSAGE);
        }

        qintptr socketDescriptor = socket->socketDescriptor();
        socket->waitForReadyRead(3000);
        data = socket->read(5);

        if (data.toStdString() == REQUEST_FILE_MESSAGE){
            readFileName(socketDescriptor);
            qDebug() << "ok, file";
        }else{
            qDebug() << "err, file";
        }
    }, Qt::DirectConnection);
}

bool Server::logIn(){
    /* read user and password on socket*/
    return true;
}

bool Server::readFileName(qintptr socketDescriptor){
    socket->waitForReadyRead(30);
    QByteArray data = socket->readAll();

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

    socket->write(OK_MESSAGE);
    if (socket->waitForBytesWritten(30)){
        qDebug() << "secondo, Ok scritto";
    }else{
        qDebug() << "Ok non scritto";
    }
}
