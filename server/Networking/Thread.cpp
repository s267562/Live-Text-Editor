#include "Thread.h"
#include <QDataStream>

class Identifier;

class Character;

Thread::Thread(QObject *parent, CRDT *crdt, QString filename, Server* server) : QThread(parent), crdt(crdt), filename(filename), server(server) {
	// Create new timer
	saveTimer = new QTimer(this);

	// Setup signal and slot
	connect(saveTimer, SIGNAL(timeout()), this, SLOT(saveCRDTToFile()));
}

void Thread::run() {
	exec();
}

void Thread::addSocket(QTcpSocket *soc, QString username) {
    std::lock_guard<std::mutex> lg(mutexSockets);
	qintptr socketDescriptor = soc->socketDescriptor();
	/* insert new socket into structure */
	sockets[socketDescriptor] = soc;
	usernames[socketDescriptor] = username;
    qDebug() << "Thread.cpp - addSocket()     sockets.size" << sockets.size();

    QMetaObject::Connection *c = new QMetaObject::Connection();
    QMetaObject::Connection *d = new QMetaObject::Connection();

	/* connect socket and signal */
	*c = connect(soc, &QAbstractSocket::readyRead, this, [this, c, d,soc]() {
        qDebug() << "                             " << soc;
		Thread::readyRead(soc, c, d);
	}, Qt::DirectConnection);

	*d = connect(soc, &QAbstractSocket::disconnected, this, [this, c, d, soc, socketDescriptor]() {
        qDebug() << "                             " << soc;
		Thread::disconnected(soc, socketDescriptor, c, d);
	}, Qt::DirectConnection);

    //writeOkMessage(soc);
    sendFile(soc);
    sendListOfUsers(soc);
    sendNewUser(soc);

    qDebug() << "                             " << socketDescriptor << " Messanger connected" << soc;
    qDebug() << ""; // newLine
}

void Thread::readyRead(QTcpSocket *soc, QMetaObject::Connection *c, QMetaObject::Connection *d) {
    if (soc->bytesAvailable() == 0) {
        return;
    }
    QByteArray data;
    if (!readChunck(soc, data, 5)) {
        /* eccezione */
        writeErrMessage(soc);
        return;
    }
    qDebug() << "Thread.cpp - readyRead()     msg received: " << data;
    qDebug() << ""; // newLine

    if (data.toStdString() == INSERT_MESSAGE) {
        if (!readInsert(soc)) {
            writeErrMessage(soc);
        }
        writeOkMessage(soc);
        readyRead(soc, c, d);
    } else if (data.toStdString() == DELETE_MESSAGE) {
        if (!readDelete(soc)) {
            writeErrMessage(soc);
        }
        readyRead(soc, c, d);
    } else if (data.toStdString() == REQUEST_FILE_MESSAGE){
        readSpace(soc);
        int fileNameSize = readNumberFromSocket(soc);
        readSpace(soc);

        QByteArray fileName;

        if (!readChunck(soc, fileName, fileNameSize)){
            writeErrMessage(soc);
            return;
        }

        if (fileName == this->filename){
            sendListOfUsers(soc);
            return;
        }

        disconnect(*c);
        disconnect(*d);
        delete c;
        delete d;
        std::shared_ptr<Thread> thread = server->getThread(fileName);
        if (thread.get() == nullptr){
            /* thread doesn't exist */
            thread = server->addThread(fileName);
        }

        qDebug() << soc->socketDescriptor() << " " <<usernames[soc->socketDescriptor()];
        sendRemoveUser(soc->socketDescriptor(),usernames[soc->socketDescriptor()]);
        thread->addSocket(soc, usernames[soc->socketDescriptor()]);
        usernames.erase(soc->socketDescriptor());
        sockets.erase(soc->socketDescriptor());


        //writeOkMessage(soc);

    }else{
        writeErrMessage(soc);
    }
}

void Thread::saveCRDTToFile() {
    if (needToSaveFile)
        crdt->saveCRDT(filename);
}

bool Thread::readInsert(QTcpSocket *soc){
    qDebug() << "Thread.cpp - readInsert()     ---------- READ INSERT ----------";

    readSpace(soc);
    int messageSize = readNumberFromSocket(soc);
    readSpace(soc);

    QByteArray characterByteFormat;
    if (!readChunck(soc, characterByteFormat, messageSize)){
        return false;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    //qDebug() << "char: "<< character.getValue();

    crdt->handleInsert(character);

    // broadcast
    this->writeInsert(soc, character);

	needToSaveFile = true;
	if (!timerStarted) {
		saveTimer->start(saveInterval);
		timerStarted = true;
	}
	return true;
}

bool Thread::readDelete(QTcpSocket *soc){
    qDebug() << "Thread.cpp - readDelete()     ---------- READ DELETE ----------";

    readSpace(soc);
    int messageSize = readNumberFromSocket(soc);
    readSpace(soc);

    QByteArray characterByteFormat;
    if (!readChunck(soc, characterByteFormat, messageSize)){
        return false;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    crdt->handleDelete(character);

    // broadcast
    this->writeDelete(soc, character);

    needToSaveFile = true;
    if (!timerStarted) {
        saveTimer->start(saveInterval);
        timerStarted = true;
    }
    return true;
}

void Thread::writeInsert(QTcpSocket *soc, Character character){
    qDebug() << "Thread.cpp - writeInsert()     ---------- WRITE INSERT ----------";

    QByteArray message(INSERT_MESSAGE);
    QByteArray characterByteFormat = character.toQByteArray();
    QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

    message.append(" " + sizeOfMessage + " " + characterByteFormat);

    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    //broadcast
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
        if(socket.first != soc->socketDescriptor()) {
            //qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
            writeMessage(socket.second, message);
        }
    }
}

void Thread::writeDelete(QTcpSocket *soc, Character character){
    qDebug() << "Thread.cpp - writeDelete()     ---------- WRITE DELETE ----------";

    QByteArray message(DELETE_MESSAGE);
    QByteArray characterByteFormat = character.toQByteArray();
    QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

    message.append(" " + sizeOfMessage + " " + characterByteFormat);

    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    //broadcast
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
        // qDebug() << "userrname of user that send the delete message: " << usernames[soc->socketDescriptor()];
        if(socket.first != soc->socketDescriptor()) {
            //qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
            writeMessage(socket.second, message);
        }
    }
}

void Thread::sendListOfUsers(QTcpSocket *soc){
    QByteArray message(LIST_OF_USERS);
    if (usernames.size() - 1 == 0){
        QByteArray usernamesSize = convertionNumber(0);
        message.append(usernamesSize);
    }else{
        QByteArray usernamesSize = convertionNumber(usernames.size() - 1);
        message.append(" " + usernamesSize);
        for (auto u : usernames){
            if (u.first != soc->socketDescriptor()){
                QByteArray usernameSize = convertionNumber(u.second.size());
                message.append(" " + usernameSize + " " + u.second.toUtf8());
            }
        }
    }

    if (!writeMessage(soc,message)){
        return;
    }
}

void Thread::sendNewUser(QTcpSocket *soc){
    QByteArray message(LIST_OF_USERS);
    QByteArray usernamesSize = convertionNumber(1);
    QByteArray usernameSize = convertionNumber(usernames[soc->socketDescriptor()].size());
    message.append(" " + usernamesSize + " " + usernameSize + " " + usernames[soc->socketDescriptor()].toUtf8());

    for (auto s : sockets){
        if (soc->socketDescriptor() != s.second->socketDescriptor()){
            if (!writeMessage(s.second, message)){
                return;
            }
        }
    }
}

void Thread::sendRemoveUser(qintptr socketDescriptor, QString username){
    QByteArray message(REMOVE_USER);
    QByteArray usernameSize = convertionNumber(username.size());
    message.append(" " + usernameSize + " " + username.toUtf8());

    for (auto s : sockets){
        if (socketDescriptor != s.first) {
            if (!writeMessage(s.second, message)) {
                return;
            }
        }
    }
}

void Thread::sendFile(QTcpSocket *soc){
    qDebug() << "Thread.cpp - sendFile()     ---------- SEND FILE ----------";
    QByteArray message(SENDING_FILE);
    const std::vector<std::vector<Character>> file = crdt->getStructure();
    QByteArray numLines = convertionNumber(file.size());

    message.append(" " + numLines);

    for (int i = 0; i < file.size(); i++){
        std::vector<Character> line = file[i];
        QByteArray numChar = convertionNumber(file[i].size());

        message.append(" " + numChar);
        for (int j = 0; j < line.size(); j++){
            Character character = line[j];
            QByteArray characterByteFormat = character.toQByteArray();
            QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

            message.append(" " + sizeOfMessage + " " + characterByteFormat);
        }
    }
    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    if (!writeMessage(soc,message)){
        return;
    }
}

void Thread::disconnected(QTcpSocket *soc, qintptr socketDescriptor, QMetaObject::Connection *c, QMetaObject::Connection *d){
    std::lock_guard<std::mutex> lg(mutexSockets);
    qDebug() << "Thread.cpp - disconnected()     " << socketDescriptor << " Disconnected";
    qDebug() << ""; // newLine
    disconnect(*c);
    disconnect(*d);
    delete c;
    delete d;
    QTcpSocket socket;
    socket.setSocketDescriptor(socketDescriptor);
    socket.deleteLater();
    sockets.erase(socketDescriptor);
    sendRemoveUser(socketDescriptor, usernames[socketDescriptor]);
    usernames.erase(socketDescriptor);
    qDebug() << usernames;
}