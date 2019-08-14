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
    //sockets.push_back(std::shared_ptr<QTcpSocket>(soc));
    sockets[socketDescriptor] = std::shared_ptr<QTcpSocket>(soc);
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
        writeOkMessage(soc);
    }else if (data.toStdString() == DELETE_MESSAGE){
        readDelete(soc);
        writeOkMessage(soc);
    }else{
        writeErrMessage(soc);
    }
}

bool Thread::readInsert(QTcpSocket *soc){
    qDebug() << "-------------INS-------------";
    soc->read(1);
    QByteArray letter = soc->read(1);
    soc->read(1);
    QByteArray size = soc->read(1);
    bool ok;
    qDebug() << " size Int:" << size.toHex().toInt(&ok,16);
    soc->read(1);
    std::vector<int> position;
    qDebug() << letter;
    
    for (int i = 0; i < size.toHex().toInt(&ok,16); i++){
        int pos = soc->read(1).toHex().toInt(&ok,16);
        position.push_back(pos);
        qDebug() << " pos:" << pos;
        if (i != size.toHex().toInt(&ok,16) - 1 || size.toHex().toInt(&ok,16) != 1){
            soc->read(1);
        }
    }
    return true;
}

bool Thread::readDelete(QTcpSocket *soc){
    qDebug() << "-------------ERR-------------";
    soc->read(1);
    QByteArray letter = soc->read(1);
    soc->read(1);
    QByteArray size = soc->read(1);
    bool ok;
    qDebug() << " size:" << size << " size Int:" << size.toHex().toInt(&ok,16);
    soc->read(1);
    std::vector<int> position;
    qDebug() << letter;

    for (int i = 0; i < size.toHex().toInt(&ok,16); i++){
        int pos = soc->read(1).toHex().toInt(&ok,16);
        position.push_back(pos);
        qDebug() << " pos:" << pos;
        if (i != size.toHex().toInt(&ok,16) - 1 || size.toHex().toInt(&ok,16) != 1){
            soc->read(1);
        }
    }
    return true;
}

void Thread::disconnected(QTcpSocket *soc){
    qDebug() << socketDescriptor << " Disconnected";

    soc->deleteLater();
    /* TO-DO: delete socket in structure */

    sockets.erase(soc->socketDescriptor());
}

bool Thread::writeOkMessage(QTcpSocket *soc){
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

bool Thread::writeErrMessage(QTcpSocket *soc){
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
