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

/**
 * this method adds a new socket to the thread
 * @param soc
 * @param username
 */
void Thread::addSocket(QTcpSocket *soc, QString username) {
    std::lock_guard<std::mutex> lg(mutexSockets);
	qintptr socketDescriptor = soc->socketDescriptor();
	/* insert new socket into structure */
	sockets[socketDescriptor] = soc;
	usernames[socketDescriptor] = username;
    qDebug() << "Thread.cpp - addSocket()     sockets.size" << sockets.size();

    QMetaObject::Connection *connectReadyRead = new QMetaObject::Connection();
    QMetaObject::Connection *connectDisconnected = new QMetaObject::Connection();

	/* connect socket and signal */
	*connectReadyRead = connect(soc, &QAbstractSocket::readyRead, this, [this, connectReadyRead, connectDisconnected,soc]() {
        qDebug() << "                             " << soc;
		Thread::readyRead(soc, connectReadyRead, connectDisconnected);
	}, Qt::DirectConnection);

	*connectDisconnected = connect(soc, &QAbstractSocket::disconnected, this, [this, connectReadyRead, connectDisconnected, soc, socketDescriptor]() {
        qDebug() << "                             " << soc;
		Thread::disconnected(soc, socketDescriptor, connectReadyRead, connectDisconnected);
	}, Qt::DirectConnection);

    sendFile(soc);
    sendListOfUsers(soc);
    sendNewUser(soc);

    qDebug() << "                             " << socketDescriptor << " Messanger connected" << soc;
    qDebug() << ""; // newLine
}

/**
 * This method allows to start reading from socket and calls different methods
 * that handles different type of message.
 */
void Thread::readyRead(QTcpSocket *soc, QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *connectDisconnected) {
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
            writeErrMessage(soc, INSERT_MESSAGE);
        }
        writeOkMessage(soc);
        readyRead(soc, connectReadyRead, connectDisconnected);
    } else if (data.toStdString() == STYLE_CAHNGED_MESSAGE) {
        if (!readStyleChanged(soc)) {
            writeErrMessage(soc);
        }
        writeOkMessage(soc);
        readyRead(soc, connectReadyRead, connectDisconnected);
    } else if (data.toStdString() == ALIGNMENT_CHANGED_MESSAGE) {
        if (!readAlignmentChanged(soc)) {
            writeErrMessage(soc);
        }
        writeOkMessage(soc);
        readyRead(soc, connectReadyRead, connectDisconnected);
    } else if (data.toStdString() == DELETE_MESSAGE) {
        if (!readDelete(soc)) {
            writeErrMessage(soc, DELETE_MESSAGE);
        }
        readyRead(soc, connectReadyRead, connectDisconnected);
    } else if (data.toStdString() == REQUEST_FILE_MESSAGE){
        readSpace(soc);
        int fileNameSize = readNumberFromSocket(soc);
        readSpace(soc);

        QByteArray fileName;

        if (!readChunck(soc, fileName, fileNameSize)){
            writeErrMessage(soc, REQUEST_FILE_MESSAGE);
            return;
        }

        if (fileName == this->filename){
            sendListOfUsers(soc);
            return;
        }

        disconnect(*connectReadyRead);
        disconnect(*connectDisconnected);
        delete connectReadyRead;
        delete connectDisconnected;
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
    }else if (data.toStdString() == SHARE_CODE) {
        if (!readShareCode(soc)) {
            writeErrMessage(soc, DELETE_MESSAGE);
        }
        readyRead(soc, connectReadyRead, connectDisconnected);
    }else{
        writeErrMessage(soc);
    }
}

void Thread::saveCRDTToFile() {
    if (needToSaveFile)
        crdt->saveCRDT(filename);
}

/**
 * This method reads the character, that was added from clients
 * @param soc
 * @return result of reading from socket
 */
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


bool Thread::readStyleChanged(QTcpSocket *soc){
    qDebug() << "Thread.cpp - readStyleChanged()     ---------- READ STYLE CHANGED ----------";

    readSpace(soc);
    int messageSize = readNumberFromSocket(soc);
    readSpace(soc);

    QByteArray characterByteFormat;
    if (!readChunck(soc, characterByteFormat, messageSize)){
        return false;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
    Character character = Character::toCharacter(jsonDocument);

    crdt->handleStyleChanged(character);

    // broadcast
    this->writeStyleChanged(soc, character);

    needToSaveFile = true;
    if (!timerStarted) {
        saveTimer->start(saveInterval);
        timerStarted = true;
    }
    return true;
}

bool Thread::readAlignmentChanged(QTcpSocket *soc){
    qDebug() << "Thread.cpp - readAlignmentChanged()     ---------- READ ALIGNMENT CHANGED ----------";


    //TODO: Manage better conversion from enum QByteArray
    readSpace(soc);
    int alignType = readNumberFromSocket(soc);
    readSpace(soc);

    int sizeBlockId = readNumberFromSocket(soc);
    
    readSpace(soc);
    
    QByteArray characterByteFormat;
    if (!readChunck(soc, characterByteFormat, sizeBlockId)){
        return false;
    }

    QJsonDocument jsonFormatBlockId = QJsonDocument::fromBinaryData(characterByteFormat);
    Character blockId = Character::toCharacter(jsonFormatBlockId);

    int row=this->crdt->getRow(blockId);

    if (!row) return false;

    crdt->handleAlignmentChanged(alignType, row);

    this->writeAlignmentChanged(soc, alignType, blockId);

    return true;
}

/**
 * This method reads the character, that was removed from other users
 * @param soc
 * @return result of reading from socket
 */
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

/**
 * This method sends the character, that was/were added
 * @param character
 * @return result of writing on socket
 */
void Thread::writeInsert(QTcpSocket *soc, Character character){
    qDebug() << "Thread.cpp - writeInsert()     ---------- WRITE INSERT ----------";

    QByteArray message(INSERT_MESSAGE);
    QByteArray characterByteFormat = character.toQByteArray();
    QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

    message.append(" " + sizeOfMessage + " " + characterByteFormat);

    qDebug() << "                         " << message;
    qDebug() << ""; // newLine


    QByteArray username=usernames[soc->socketDescriptor()].toUtf8();
    QByteArray sizeOfSender=convertionNumber(username.size());
    message.append(" "+sizeOfSender+" "+username);
    qDebug() << "msg:" << " "+sizeOfSender+" "+username;

    //broadcast
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
        if(socket.first != soc->socketDescriptor()) {
            qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
            writeMessage(socket.second, message);
        }
    }
}

void Thread::writeStyleChanged(QTcpSocket *soc, Character character){
    qDebug() << "Thread.cpp - writeStyleChanged()     ---------- WRITE STYLE CHANGED ----------";

    QByteArray message(STYLE_CAHNGED_MESSAGE);
    QByteArray characterByteFormat = character.toQByteArray();
    QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

    message.append(" " + sizeOfMessage + " " + characterByteFormat);

    QByteArray username=usernames[soc->socketDescriptor()].toUtf8();
    QByteArray sizeOfSender=convertionNumber(username.size());
    message.append(" "+sizeOfSender+" "+username);

    qDebug() << "msg:" << " "+sizeOfSender+" "+username;


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


void Thread::writeAlignmentChanged(QTcpSocket *soc, int alignment, Character blockId){

    qDebug() << "Thread.cpp - writeAlignmentChanged()     ---------- WRITE ALIGNMENT CHANGED ----------";

    QByteArray message(ALIGNMENT_CHANGED_MESSAGE);

    QByteArray alignmentByteFormat=convertionNumber(alignment);
    QByteArray blockIdByteFormat=blockId.toQByteArray();
    //QByteArray sizeOfMessageAt = convertionNumber(alignmentByteFormat.size());
    QByteArray sizeBlockId = convertionNumber(blockIdByteFormat.size());


    message.append(" " + alignmentByteFormat);
    message.append(" " + sizeBlockId);
    message.append(" " + blockIdByteFormat);


    QByteArray username=usernames[soc->socketDescriptor()].toUtf8();
    QByteArray sizeOfSender=convertionNumber(username.size());

    message.append(" "+sizeOfSender+" "+username);

    qDebug() << "msg:" << " "+sizeOfSender+" "+username;
    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    //broadcast
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
        // qDebug() << "userrname of user that send the delete message: " << usernames[soc->socketDescriptor()];
        //if(socket.first != soc->socketDescriptor()) {
            //qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
            writeMessage(socket.second, message);
       // }
    }


}

/**
 * This method sends the character, that was removed
 * @param character
 * @return result of writing on socket
 */
void Thread::writeDelete(QTcpSocket *soc, Character character){
    qDebug() << "Thread.cpp - writeDelete()     ---------- WRITE DELETE ----------";

    QByteArray message(DELETE_MESSAGE);
    QByteArray characterByteFormat = character.toQByteArray();
    QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

    message.append(" " + sizeOfMessage + " " + characterByteFormat);

    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    QByteArray username=usernames[soc->socketDescriptor()].toUtf8();
    QByteArray sizeOfSender=convertionNumber(username.size());

    message.append(" "+sizeOfSender+" "+username);
    qDebug() << "msg:" << " "+sizeOfSender+" "+username;

    //broadcast
    for(std::pair<qintptr, QTcpSocket*> socket : sockets){
         qDebug() << "userrname of user that send the delete message: " << usernames[soc->socketDescriptor()];
        if(socket.first != soc->socketDescriptor()) {
            qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
            writeMessage(socket.second, message);
        }
    }
}

/**
 * This method sends the list of users, that are working on this file
 * @param soc
 */
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

/**
 * This method sends the new username, which has just been added
 * @param soc
 */
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

/**
 * This method sends the username, which was disconnected
 * @param socketDescriptor
 * @param username
 */
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

/**
 * This method sends all file to user
 * @param soc
 */
void Thread::sendFile(QTcpSocket *soc){
    qDebug() << "Thread.cpp - sendFile()     ---------- SEND FILE ----------";
    QByteArray message(SENDING_FILE);
    const std::vector<std::vector<Character>> file = crdt->getStructure();
    std::vector<int> blockFormat;


    for(int i=0; i<this->crdt->getTextDocument()->blockCount(); i++){
        QTextBlock tb=this->crdt->getTextDocument()->findBlockByNumber(i);
        int al = tb.blockFormat().alignment();

        blockFormat.push_back(al);

//        if(al.testFlag(Qt::AlignLeft)){
//            blockFormat.push_back(LEFT);
//        }
//        else if(al.testFlag(Qt::AlignRight)){
//            blockFormat.push_back(RIGHT);
//        }
//        else if(al.testFlag(Qt::AlignHCenter)){
//            blockFormat.push_back(CENTER);
//        }
//        else if(al.testFlag(Qt::AlignJustify)){
//            blockFormat.push_back(JUSTIFY);
//        }
//        else{
//            //Allineamento non riconosciuto
//            qDebug() << "Allineamento non riconosciuto" ;
//            //TODO: Gestire
//        }
    }


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

    QByteArray numBlocks = convertionNumber(blockFormat.size());
    message.append(" " + numBlocks);

    for (int i = 0; i < blockFormat.size(); i++){
        QByteArray alignment = convertionNumber(blockFormat.at(i));
        message.append(" " + alignment);
    }

    qDebug() << "                         " << message;
    qDebug() << ""; // newLine

    if (!writeMessage(soc,message)){
        return;
    }

}

bool Thread::readShareCode(QTcpSocket *soc){
    qDebug() << "Thread.cpp - readShareCode()     ---------- READ SHARECODE ----------";
    readSpace(soc);
    int shareCodeSize = readNumberFromSocket(soc);
    readSpace(soc);

    QByteArray shareCode;
    if (!readChunck(soc, shareCode, shareCodeSize)){
        return false;
    }

    qDebug() << shareCodeSize << shareCode;
    QString filename;

    if (server->handleShareCode(usernames[soc->socketDescriptor()], shareCode, filename)){
        sendAddFile(soc, filename);
    }else{
        writeErrMessage(soc, SHARE_CODE);
    }

    return true;
}

bool Thread::sendAddFile(QTcpSocket *soc, QString filename) {
    QByteArray message(ADD_FILE);

    QByteArray fileNameSize = convertionNumber(filename.size());
    QByteArray shared;
    shared.setNum(0);
    message.append(" " + fileNameSize + " " + filename.toUtf8() + " " + shared);

    if (!writeMessage(soc, message)) {
        return false;
    }

    return true;
}

void Thread::disconnected(QTcpSocket *soc, qintptr socketDescriptor, QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *connectDisconnected){
    std::lock_guard<std::mutex> lg(mutexSockets);
    qDebug() << "Thread.cpp - disconnected()     " << socketDescriptor << " Disconnected";
    qDebug() << ""; // newLine
    disconnect(*connectReadyRead);
    disconnect(*connectDisconnected);
    delete connectReadyRead;
    delete connectDisconnected;
    QTcpSocket socket;
    socket.setSocketDescriptor(socketDescriptor);
    socket.deleteLater();
    sockets.erase(socketDescriptor);
    sendRemoveUser(socketDescriptor, usernames[socketDescriptor]);
    usernames.erase(socketDescriptor);
    qDebug() << usernames;
}

