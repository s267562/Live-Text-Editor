#include "Thread.h"

Thread::Thread(QObject *parent):QThread(parent){}

void Thread::run(){
    exec();
}

void Thread::addSocket(qintptr socketDescriptor){
    QTcpSocket *soc = new QTcpSocket();

    // set the ID
    if(!soc->setSocketDescriptor(socketDescriptor)){
        // something's wrong, we just emit a signal
        emit error(soc->error());
        return;
    }

    /* insert new socket into structure */
    sockets.push_back(std::shared_ptr<QTcpSocket>(soc));
    qDebug() << "size" << sockets.size();

    /* connect socket and signal */
    connect(soc, &QAbstractSocket::readyRead, this, [this,soc](){
        qDebug() << soc;
        Thread::readyRead(soc);
    }, Qt::DirectConnection);

    connect(soc, &QAbstractSocket::disconnected, this, [this,soc](){
        qDebug() << soc;
        Thread::disconnected(soc);
    });

    qDebug() << socketDescriptor << " Client connected" << soc;
}

void Thread::readyRead(QTcpSocket *soc){
    //soc->waitForReadyRead(300);
    QByteArray data = soc->read(5);

    // will write on server side window
    qDebug() << socketDescriptor << " Data in: " << data;

    if (data.toStdString() == INSERT_MESSAGE){
        readInsert(soc);
        soc->write(OK_MESSAGE);
    }else if (data.toStdString() == DELETE_MESSAGE){
        readDelete(soc);
        soc->write(OK_MESSAGE);
    }
}

bool Thread::readInsert(QTcpSocket *soc){
    qDebug() << "ok, ins";
    QByteArray size = soc->read(1);
    soc->read(1);
    QByteArray letter = soc->read(1);
    soc->read(1);
    QByteArray pos = soc->read(1);
    qDebug() << letter << " pos: " << pos;
    return true;
}

bool Thread::readDelete(QTcpSocket *soc){
    qDebug() << "ok, del";
    QByteArray size = soc->read(1);
    soc->read(1);
    QByteArray letter = soc->read(1);
    soc->read(1);
    QByteArray pos = soc->read(1);
    qDebug() << letter << " pos: " << pos;
    return true;
}

void Thread::disconnected(QTcpSocket *soc){
    qDebug() << socketDescriptor << " Disconnected";

    soc->deleteLater();
    /* TO-DO: delete socket in structure */
}
