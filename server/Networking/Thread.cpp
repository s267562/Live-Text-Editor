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

void Thread::addSocket(QTcpSocket *soc) {
    std::lock_guard<std::mutex> lg(mutexSockets);
	qintptr socketDescriptor = soc->socketDescriptor();
	/* insert new socket into structure */
	sockets[socketDescriptor] = std::shared_ptr<QTcpSocket>(soc);
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

    writeOkMessage(soc);

    qDebug() << "                             " << socketDescriptor << " Client connected" << soc;
    qDebug() << ""; // newLine
}

bool Thread::readInsert(QTcpSocket *soc){
    qDebug() << "Thread.cpp - readInsert()     ---------- READ INSERT ----------";

	readSpace(soc);
	int sizeString = readNumberFromSocket(soc);
	readSpace(soc);

	QByteArray letter;
	if (!readChunck(soc, letter, sizeString)){
		return false;
	}
	readSpace(soc);

	//siteID
	int sizeSiteId = readNumberFromSocket(soc);
	readSpace(soc);

	QByteArray siteId;
	if (!readChunck(soc, siteId, sizeSiteId)){
		return false;
	}
	readSpace(soc);

	int posChInt = readNumberFromSocket(soc);
	readSpace(soc);

	int posLineInt = readNumberFromSocket(soc);

    qDebug() << "                              ch: "<<letter << "siteId: " << siteId << " posCh: " << posChInt << " posLine: " << posLineInt;
    qDebug() << ""; // newLine

    Pos startPos{posChInt, posLineInt};

    for(char c : letter) {
        Character character = crdt->handleInsert(c, startPos, QString{siteId});
        this->insert(QString{character.getValue()}, character.getSiteId(), character.getPosition());

        // increment startPos
        if(c == '\n') {
            startPos.resetCh();
            startPos.incrementLine();
        } else {
            startPos.incrementCh();
        }
    }

	needToSaveFile = true;
	if (!timerStarted) {
		saveTimer->start(saveInterval);
		timerStarted = true;
	}
	return true;
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
        readyRead(soc, c, d);
    } else if (data.toStdString() == DELETE_MESSAGE) {
        if (!readDelete(soc)) {
            writeErrMessage(soc);
        }
        readyRead(soc, c, d);
    } else if (data.toStdString() == REQUEST_FILE_MESSAGE){
        disconnect(*c);
        disconnect(*d);
        delete c;
        delete d;

        readSpace(soc);
        int fileNameSize = readNumberFromSocket(soc);
        readSpace(soc);

        QByteArray fileName;
        if (!readChunck(soc, fileName, fileNameSize)){
            writeErrMessage(soc);
            return;
        }
        std::shared_ptr<Thread> thread = server->getThread(fileName);
        if (thread.get() == nullptr){
            /* thread doesn't exist */
            thread = server->addThread(fileName);
        }
        qDebug() << soc->socketDescriptor();
        thread->addSocket(soc);

        //writeOkMessage(soc);
    }else{
		writeErrMessage(soc);
	}
}

void Thread::saveCRDTToFile() {
	if (needToSaveFile)
		crdt->saveCRDT(filename);
}

bool Thread::readDelete(QTcpSocket *soc){
    qDebug() << "Thread.cpp - readDelete()     ---------- READ DELETE ----------";

    readSpace(soc);
    QByteArray letter;
    if (!readChunck(soc, letter, 1)){
        return false;
    }
    readSpace(soc);

    //siteID
    int sizeSiteId = readNumberFromSocket(soc);
    readSpace(soc);

    QByteArray siteId;
    if (!readChunck(soc, siteId, sizeSiteId)){
        return false;
    }
    readSpace(soc);

    int size = readNumberFromSocket(soc);
    qDebug() << "                              size:" << size << " size Int:" << size;
    readSpace(soc);
    std::vector<Identifier> position;
    qDebug() << "                             " << letter;

    for (int i = 0; i < size; i++){
        int pos = readNumberFromSocket(soc);
        Identifier identifier(pos, siteId);
        position.push_back(identifier);
        qDebug() << "                              pos:" << pos;
        if (i != size - 1 || size != 1){
            readSpace(soc);
        }
    }
    qDebug() << ""; // newLine

    Character character(letter[0], 0, siteId, position);

    crdt->handleDelete(character);

    // broadcast
    this->deleteChar(QString{character.getValue()}, character.getSiteId(), character.getPosition());

	needToSaveFile = true;
	if (!timerStarted) {
		saveTimer->start(saveInterval);
		timerStarted = true;
	}
    return true;
}

void Thread::insert(QString str, QString siteId,std::vector<Identifier> pos){
    qDebug() << "Thread.cpp - insert()     ---------- WRITE INSERT ----------";

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
    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    //broadcast
    for(std::pair<qintptr, std::shared_ptr<QTcpSocket>> socket : sockets){
        writeMessage(socket.second.get(), message);
    }
}

void Thread::deleteChar(QString str,  QString siteId, std::vector<Identifier> pos){
    qDebug() << "Thread.cpp - deleteChar()     ---------- WRITE DELETE ----------";

    QByteArray message(DELETE_MESSAGE);
    QByteArray siteIdSize = convertionNumber(siteId.size());
    QByteArray posSize = convertionNumber(pos.size());

    qDebug() << "                              pos.size: " << pos.size();
    message.append(" " + str.toUtf8() + " " + siteIdSize + " " + siteId.toUtf8() + " "+ posSize + " ");
    QByteArray position;

    for (int i = 0; i < pos.size(); i++){
        position.append(convertionNumber(pos[i].getDigit()));
        if (i != pos.size() - 1 || pos.size() != 1){
            position.append(" ");
        }
    }
    message.append(position);
    qDebug() << "                             " << message;
    qDebug() << ""; // newLine

    //broadcast
    for(std::pair<qintptr, std::shared_ptr<QTcpSocket>> socket : sockets){
        writeMessage(socket.second.get(), message);
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
    sockets.erase(soc->socketDescriptor());
}
