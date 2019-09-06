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
    readSpace(soc);

    int boldInt = readNumberFromSocket(soc);
    bool bold = boldInt == 1;
    readSpace(soc);

    int italicInt = readNumberFromSocket(soc);
    bool italic = italicInt == 1;
    readSpace(soc);

    int underlineInt = readNumberFromSocket(soc);
    bool underline = underlineInt == 1;

    CharFormat charFormat;
    charFormat.setBold(bold);
    charFormat.setItalic(italic);
    charFormat.setUnderline(underline);

    qDebug() << "                              ch: "<<letter << "siteId: " << siteId << " posCh: " << posChInt << " posLine: " << posLineInt;
    qDebug() << ""; // newLine

    Pos startPos{posChInt, posLineInt};

    for(char c : letter) {
        Character character = crdt->handleInsert(c, charFormat, startPos, QString{siteId});
        //this->insert(QString{character.getValue()}, character.getSiteId(), character.getPosition());
        this->insert(character);

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

bool Thread::readDelete(QTcpSocket *soc){
    qDebug() << "Thread.cpp - readDelete()     ---------- READ DELETE ----------";

    readSpace(soc);
    QByteArray letter;
    if (!readChunck(soc, letter, 1)){
        return false;
    }
    readSpace(soc);

    // charFormat is not important when delete char.
    CharFormat charFormat;

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

    Character character(letter[0], charFormat, 0, siteId, position);

    crdt->handleDelete(character);

    // broadcast
    //this->deleteChar(QString{character.getValue()}, character.getSiteId(), character.getPosition());
    this->deleteChar(character);

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

    // TODO add charFormat...

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
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
        writeMessage(socket.second, message);
    }
}

void Thread::insert(Character character){
    qDebug() << "Thread.cpp - insert()     ---------- WRITE INSERT ----------";

    QByteArray message(INSERT_MESSAGE);
    QByteArray characterByteFormat = character.toQByteArray();
    QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

    message.append(" " + sizeOfMessage + " " + characterByteFormat);

    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    //broadcast
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
        writeMessage(socket.second, message);
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
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
        writeMessage(socket.second, message);
    }
}

void Thread::deleteChar(Character character){
    qDebug() << "Thread.cpp - insert()     ---------- WRITE DELETE ----------";

    QByteArray message(DELETE_MESSAGE);
    QByteArray characterByteFormat = character.toQByteArray();
    QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

    message.append(" " + sizeOfMessage + " " + characterByteFormat);

    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    //broadcast
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
        writeMessage(socket.second, message);
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

void Thread::sendFile(QTcpSocket *soc){
    qDebug() << "Thread.cpp - sendFile()     ---------- SEND FILE ----------";
    QByteArray message(SENDING_FILE);
    const std::vector<std::vector<Character>> file = crdt->getStructure();
    QByteArray numLines = convertionNumber(file.size());

    message.append(" " + numLines);

    /*for (int i = 0; i < file.size(); i++){
        std::vector<Character> line = file[i];
        QByteArray numChar = convertionNumber(file[i].size());

        message.append(" " + numChar);
        for (int j = 0; j < line.size(); j++){
            QString str(line[j].getValue());
            QString siteId = line[j].getSiteId();
            std::vector<Identifier> pos = line[j].getPosition();

            QByteArray strSize = convertionNumber(str.size());
            QByteArray siteIdSize = convertionNumber(siteId.size());
            QByteArray posSize = convertionNumber(pos.size());

            message.append(" " + strSize + " " + str.toUtf8() + " " + siteIdSize + " " + siteId.toUtf8() + " " + posSize + " ");
            QByteArray position;

            for (int k = 0; k < pos.size(); k++){
                position.append(convertionNumber(pos[i].getDigit()));
                if (k != pos.size() - 1 || pos.size() != 1){
                    position.append(" ");
                }
            }
            message.append(position);

        }
    }*/

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