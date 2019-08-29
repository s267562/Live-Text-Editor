#include "Thread.h"
#include <QDataStream>

class Identifier;
class Character;

Thread::Thread(QObject *parent, CRDT *crdt) : QThread(parent), crdt(crdt) {}

void Thread::run(){
    exec();
}

void Thread::addSocket(QTcpSocket *soc) {
    qintptr socketDescriptor = soc->socketDescriptor();
    /* insert new socket into structure */
    sockets[socketDescriptor] = std::shared_ptr<QTcpSocket>(soc);
    qDebug() << "size" << sockets.size();

    /* connect socket and signal */
    connect(soc, &QAbstractSocket::readyRead, this, [this,soc](){
        qDebug() << soc;
        Thread::readyRead(soc);
    }, Qt::DirectConnection);

    connect(soc, &QAbstractSocket::disconnected, this, [this,soc, socketDescriptor](){
        qDebug() << soc;
        Thread::disconnected(soc,socketDescriptor);
    }, Qt::DirectConnection);

    qDebug() << socketDescriptor << " Client connected" << soc;
}

void Thread::readyRead(QTcpSocket *soc){
    QByteArray data;
    if (!readChunck(soc, data, 5)){
        /* eccezione */
        writeErrMessage(soc);
        return;
    }

    if (data.toStdString() == INSERT_MESSAGE){
        if (readInsert(soc)){
            writeOkMessage(soc);
        }else{
            writeErrMessage(soc);
        }
    }else if (data.toStdString() == DELETE_MESSAGE){
        if (readDelete(soc)){
            writeOkMessage(soc);
        }else{
            writeErrMessage(soc);
        }
    }else{
        writeErrMessage(soc);
    }
}

bool Thread::readInsert(QTcpSocket *soc){
    qDebug() << "-------------READ INSERT-------------";
    soc->read(1);                       // " "
    int sizeString = readNumberFromSocket(soc);
    soc->read(1);                       // " "

    QByteArray letter;
    if (!readChunck(soc, letter, sizeString)){
        return false;
    }
    soc->read(1);                       // " "

    //siteID
    int sizeSiteId = readNumberFromSocket(soc);
    soc->read(1);                       // " "

    QByteArray siteId;
    if (!readChunck(soc, siteId, sizeSiteId)){
        return false;
    }
    soc->read(1);                       // " "

    int posChInt = readNumberFromSocket(soc);
    soc->read(1);                       // " "

    int posLineInt = readNumberFromSocket(soc);

    qDebug() << "ch: "<<letter << "siteId: " << siteId << " posCh: " << posChInt << " posLine: " << posLineInt;

    Pos startPos{posChInt, posLineInt};

    for(int i=letter.size(); i>0; i--) {
        char c = letter[i-1];
        Character character = crdt->handleInsert(c, startPos, QString{siteId});
        // send character (broadcast)
        this->insert(QString{character.getValue()}, character.getSiteId(), character.getPosition());
    }
    return true;
}

bool Thread::readDelete(QTcpSocket *soc){
    qDebug() << "-------------DEL-------------";
    bool ok;
    soc->read(1);
    QByteArray letter;
    if (!readChunck(soc, letter, 1)){
        return false;
    }
    soc->read(1);

    //siteID
    QByteArray sizeSiteId;
    if (!readChunck(soc, sizeSiteId, 1)){
        return false;
    }
    soc->read(1);

    QByteArray siteId;
    if (!readChunck(soc, siteId, sizeSiteId.toHex().toInt(&ok,16))){
        return false;
    }
    soc->read(1);

    QByteArray size = soc->read(1);
    qDebug() << " size:" << size << " size Int:" << size.toHex().toInt(&ok,16);
    soc->read(1);
    std::vector<Identifier> position;
    qDebug() << letter;

    for (int i = 0; i < size.toHex().toInt(&ok,16); i++){
        int pos = soc->read(1).toHex().toInt(&ok,16);
        Identifier identifier(pos, siteId);
        position.push_back(identifier);
        qDebug() << " pos:" << pos;
        if (i != size.toHex().toInt(&ok,16) - 1 || size.toHex().toInt(&ok,16) != 1){
            soc->read(1);
        }
    }

    Character character(letter[0], 0, siteId, position);
    // Message message(character, soc->socketDescriptor(), DELETE);
    // messagesQueue.push(message);
    // emit newMessage();

    crdt->handleDelete(character);

    // send character (broadcast)
    this->deleteChar(QString{character.getValue()}, character.getSiteId(), character.getPosition());

    return true;
}

void Thread::insert(QString str, QString siteId,std::vector<Identifier> pos){
    qDebug() << "-------------WRITE INSERT-------------";
    QByteArray message(INSERT_MESSAGE);
    QByteArray strSize = convertionNumber(str.size());
    QByteArray siteIdSize = convertionNumber(siteId.size());
    QByteArray posSize = convertionNumber(pos.size());

    message.append(" " + strSize + " " + str.toUtf8() + " " + siteIdSize + " " + siteId.toUtf8() + " " + posSize + " ");
    QByteArray position;

    for (int i = 0; i < pos.size(); i++){
        position.append(convertionNumber(pos[i].getDigit()));
        if (i != pos.size() - 1 || pos.size() != 1){
            position.append(" ");
        }
    }
    message.append(position);
    qDebug() << message;

    //broadcast
    for(std::pair<qintptr, std::shared_ptr<QTcpSocket>> socket : sockets){
        socket.second->write(message);
        std::cout << "msg sent\n";
    }
}

void Thread::deleteChar(QString str, QString siteId, std::vector<int> pos){
    QByteArray message(DELETE_MESSAGE);
    QByteArray data;
    QDataStream in(&data,  QIODevice::WriteOnly);
    data.append(" ");
    in << str.size();

    data.append(" " + str + " " + siteId.size() + " " + siteId + " "+ pos.size() + " ");
    QByteArray position;

    for (int i = 0; i < pos.size(); i++){
        position.append(pos[i]);
        if (i != pos.size() - 1 || pos.size() != 1){
            position.append(" ");
        }
    }
    data.append(position);
    message.append(data);
    qDebug() << message;

    //broadcast
    for(std::pair<qintptr, std::shared_ptr<QTcpSocket>> socket : sockets){
        socket.second->write(message);
    }
}

void Thread::deleteChar(QString str,  QString siteId, std::vector<Identifier> pos){
    QByteArray message(DELETE_MESSAGE);
    QByteArray data;
    QDataStream in(&data,  QIODevice::WriteOnly);
    data.append(" ");
    in << str.size();

    data.append(" " + str + " " + siteId.size() + " " + siteId + " "+ pos.size() + " ");
    QByteArray position;

    for (int i = 0; i < pos.size(); i++){
        position.append(pos[i].getDigit());
        if (i != pos.size() - 1 || pos.size() != 1){
            position.append(" ");
        }
    }
    data.append(position);
    message.append(data);
    qDebug() << message;

    //broadcast
    for(std::pair<qintptr, std::shared_ptr<QTcpSocket>> socket : sockets){
        socket.second->write(message);
    }
}

void Thread::disconnected(QTcpSocket *soc, qintptr socketDescriptor){
    qDebug() << socketDescriptor << " Disconnected";
    QTcpSocket socket;
    socket.setSocketDescriptor(socketDescriptor);
    socket.deleteLater();
    sockets.erase(soc->socketDescriptor());
}
