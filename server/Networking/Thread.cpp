#include "Thread.h"
#include <QDataStream>

class Identifier;

class Character;

Thread::Thread(QObject *parent, CRDT *crdt, QString filename, QString usernameOwner, Server *server) :
		QThread(parent), crdt(crdt), filename(filename), usernameOwner(usernameOwner), server(server) {

	// Create new timer
	saveTimer = new QTimer(nullptr);
	saveTimer->moveToThread(this);
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
bool Thread::addSocket(QTcpSocket *soc, QString username) {
	qDebug() << "Thread.cpp - addSocket()     ---------- ADD SOCKET ----------";
	qintptr socketDescriptor = soc->socketDescriptor();
	usernames[socketDescriptor] = username;
	if (!sendFile(soc)) {
		usernames.erase(socketDescriptor);
		return false;
	}

	if (!sendListOfUsers(soc)) {
		usernames.erase(socketDescriptor);
		return false;
	}

	if (!sendNewUser(soc)) {
		usernames.erase(socketDescriptor);
		return false;
	}

	/* insert new socket into structure */
	sockets[socketDescriptor] = soc;
	sockets[socketDescriptor]->setParent(nullptr);
	sockets[socketDescriptor]->moveToThread(this);
	pendingSocket.erase(socketDescriptor);

	QMetaObject::Connection *connectReadyRead = new QMetaObject::Connection();
	QMetaObject::Connection *connectDisconnected = new QMetaObject::Connection();

	/* connect socket and signal */
	connectSlot(soc, connectReadyRead, connectDisconnected);

	qDebug() << "                             " << socketDescriptor << " Messanger connected" << soc;
	qDebug() << ""; // newLine
	return true;
}

void Thread::connectSlot(QTcpSocket *soc, QMetaObject::Connection *connectReadyRead,
						 QMetaObject::Connection *connectDisconnected) {
	qintptr socketDescriptor = soc->socketDescriptor();
	*connectReadyRead = connect(soc, &QAbstractSocket::readyRead, this,
								[this, connectReadyRead, connectDisconnected, soc]() {
									qDebug() << "                             " << soc;
									Thread::readyRead(soc, connectReadyRead, connectDisconnected);
								}, Qt::QueuedConnection);

	*connectDisconnected = connect(soc, &QAbstractSocket::disconnected, this,
								   [this, connectReadyRead, connectDisconnected, soc, socketDescriptor]() {
									   qDebug() << "                             " << soc;
									   Thread::disconnected(soc, socketDescriptor, connectReadyRead,
															connectDisconnected);
								   }, Qt::QueuedConnection);

}

/**
 * This method allows to start reading from socket and calls different methods
 * that handles different type of message.
 */
void Thread::readyRead(QTcpSocket *soc, QMetaObject::Connection *connectReadyRead,
					   QMetaObject::Connection *connectDisconnected) {
	if (soc == nullptr)
		return;

	if (soc->bytesAvailable() == 0) {
		return;
	}
	QByteArray data;
	if (!readChunck(soc, data, 5)) {
		writeErrMessage(soc);
		return;
	}

	qDebug() << "Thread.cpp - readyRead()     msg received: " << data;
	qDebug() << ""; // newLine
	std::shared_lock<std::shared_mutex> fileDeletedMutex(mutexFileDeleted);
	if (data.toStdString() == INSERT_MESSAGE && !fileDeleted) {
		std::unique_lock<std::shared_mutex> needToSaveMutex(mutexNeedToSave);
		if (!readInsert(soc)) {
			writeErrMessage(soc, INSERT_MESSAGE);
			return;
		}
		writeOkMessage(soc);
		needToSaveMutex.unlock();
		//readyRead(soc, connectReadyRead, connectDisconnected);
	} else if (data.toStdString() == STYLE_CAHNGED_MESSAGE && !fileDeleted) {
		std::unique_lock<std::shared_mutex> usernamesLock(mutexUsernames);
		std::unique_lock<std::shared_mutex> needToSaveMutex(mutexNeedToSave);
		if (!readStyleChanged(soc)) {
			writeErrMessage(soc);
			return;
		}
		writeOkMessage(soc);
		needToSaveMutex.unlock();
		//readyRead(soc, connectReadyRead, connectDisconnected);
	} else if (data.toStdString() == DELETE_MESSAGE && !fileDeleted) {
		std::unique_lock<std::shared_mutex> usernamesLock(mutexUsernames);
		std::unique_lock<std::shared_mutex> needToSaveMutex(mutexNeedToSave);
		if (!readDelete(soc)) {
			writeErrMessage(soc, DELETE_MESSAGE);
			return;
		}
		writeOkMessage(soc);
		needToSaveMutex.unlock();
		//readyRead(soc, connectReadyRead, connectDisconnected);
	} else if (data.toStdString() == ALIGNMENT_CHANGED_MESSAGE) {
		std::unique_lock<std::shared_mutex> socketsLock(mutexSockets);
		std::shared_lock<std::shared_mutex> usernamesLock(mutexUsernames);
		std::unique_lock<std::shared_mutex> needToSaveMutex(mutexNeedToSave);
		if (!readAlignmentChanged(soc)) {
			writeErrMessage(soc);
			return;
		}
		writeOkMessage(soc);
		needToSaveMutex.unlock();
		//readyRead(soc, connectReadyRead, connectDisconnected);
	} else if (data.toStdString() == REQUEST_FILE_MESSAGE) {
		std::unique_lock<std::shared_mutex> socketsLock(mutexSockets);
		std::unique_lock<std::shared_mutex> pendingSocketsLock(mutexPendingSockets);
		std::unique_lock<std::shared_mutex> usernamesLock(mutexUsernames);
		std::shared_lock<std::shared_mutex> filenameLock(mutexFilename);

		std::unique_lock<std::shared_mutex> threadsMutex(server->mutexThread);
		if (!readFileName(soc, connectReadyRead, connectDisconnected)) {
			writeErrMessage(soc, REQUEST_FILE_MESSAGE);
			return;
		}
	} else if (data.toStdString() == SHARE_CODE) {
		std::shared_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
		if (!readShareCode(soc)) {
			writeErrMessage(soc, DELETE_MESSAGE);
			return;
		}
		readyRead(soc, connectReadyRead, connectDisconnected);
	} else if (data.toStdString() == EDIT_ACCOUNT) {
		QString oldUsername = usernames[soc->socketDescriptor()];
		std::unique_lock<std::shared_mutex> allUsernamesMutex(server->mutexAllUsernames);
		std::unique_lock<std::shared_mutex> threadsMutex(server->mutexThread);
		std::unique_lock<std::shared_mutex> socketsMutex(server->mutexSockets);
		std::unique_lock<std::shared_mutex> usernamesMutex(mutexUsernames);

		if (readEditAccount(soc)) {
			sendUser(soc);
			server->sendFileNames(soc);
			if (oldUsername != usernames[soc->socketDescriptor()]) {
				sendRemoveUser(soc->socketDescriptor(), oldUsername);
				sendNewUser(soc);
			}
		} else {
			writeErrMessage(soc, EDIT_ACCOUNT);
		}
	} else if (data.toStdString() == REQUEST_USERNAME_LIST_MESSAGE) {
		if (!readRequestUsernameList(soc)) {
			writeErrMessage(soc, REQUEST_USERNAME_LIST_MESSAGE);
		}
	} else if (data.toStdString() == FILE_INFORMATION_CHANGES) {
		std::shared_lock<std::shared_mutex> allUsernamesMutex(server->mutexAllUsernames);
		std::unique_lock<std::shared_mutex> serverSocketsMutex(server->mutexSockets);
		std::unique_lock<std::shared_mutex> threadSocketsMutex(mutexSockets);
		std::unique_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
		std::unique_lock<std::shared_mutex> pendingSocketsMutex(mutexPendingSockets);

		if (!readFileInformationChanges(soc)) {
			writeErrMessage(soc, FILE_INFORMATION_CHANGES);
		}
	} else if (data.toStdString() == DELETE_FILE) {
		std::shared_lock<std::shared_mutex> allUsernamesMutex(server->mutexAllUsernames);
		std::unique_lock<std::shared_mutex> threadsMutex(server->mutexThread);
		std::unique_lock<std::shared_mutex> serverSocketsMutex(server->mutexSockets);

		std::unique_lock<std::shared_mutex> threadSocketsMutex(mutexSockets);
		std::shared_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
		std::unique_lock<std::shared_mutex> pendingSocketsMutex(mutexPendingSockets);

		if (!readDeleteFile(soc)) {
			writeErrMessage(soc, DELETE_FILE);
		}
	} else {
		writeErrMessage(soc);
	}
}

/**
 * Save CRDT to a file
 */
void Thread::saveCRDTToFile() {
	if (crdt == nullptr)
		return;
	std::shared_lock<std::shared_mutex> filenameLock(mutexFilename);
	std::unique_lock<std::shared_mutex> needToSaveLock(mutexNeedToSave);

	QString jsonFileName = filename;
	if (needToSaveFile) {
		qDebug() << "Saving CRDT for file: " + jsonFileName;
		crdt->saveCRDT(jsonFileName);
		needToSaveFile = false;
	}
}

/**
 * This method handles a file request
 * @param soc
 * @param connectReadyRead
 * @param connectDisconnected
 * @return
 */
bool Thread::readFileName(QTcpSocket *soc, QMetaObject::Connection *connectReadyRead,
						  QMetaObject::Connection *connectDisconnected) {
	qDebug() << "Thread.cpp - readFileName()     ---------- READ FILENAME ----------";
	if (!readSpace(soc)) {
		return false;
	}

	/* fileNameSize */
	int fileNameSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	/* jsonFileName */
	QString jsonFileName;
	if (!readQString(soc, jsonFileName, fileNameSize)) {
		writeErrMessage(soc, REQUEST_FILE_MESSAGE);
		return false;
	}

	if (jsonFileName == this->filename && !fileDeleted) {
		if (pendingSocket.find(soc->socketDescriptor()) != pendingSocket.end()) {
			sockets[soc->socketDescriptor()] = soc;
			pendingSocket.erase(soc->socketDescriptor());
			if (!sendFile(soc))
				return false;
			if (!sendListOfUsers(soc))
				return false;
			if (!sendNewUser(soc))
				return false;
		} else {
			if (!sendFile(soc))
				return false;
			if (!sendListOfUsers(soc))
				return false;
			if (!sendNewUser(soc))
				return false;
		}
		return true;
	}

	QStringList fields = jsonFileName.split("%_##$$$##_%");
	if (fields.size() < 2) {
		QString owner = usernames[soc->socketDescriptor()];
		jsonFileName = owner + "%_##$$$##_%" + fields[0];
	} else {
		QString owner = fields[0];
	}

	disconnect(*connectReadyRead);
	disconnect(*connectDisconnected);

	std::shared_ptr<Thread> thread = server->getThread(jsonFileName);

	if (!sendRemoveUser(soc->socketDescriptor(), usernames[soc->socketDescriptor()])) {
		return false;
	}
	if (thread.get() == nullptr) {
		/* thread doesn't exist */
		thread = server->addThread(jsonFileName, usernames[soc->socketDescriptor()]);
		thread->addSocket(soc, usernames[soc->socketDescriptor()]);
		thread->moveToThread(thread.get());
		thread->start();
	} else {
		if (thread.get() != this) {
			std::unique_lock<std::shared_mutex> socketsLock(thread->mutexSockets);
			std::unique_lock<std::shared_mutex> pendingSocketsLock(thread->mutexPendingSockets);
			std::unique_lock<std::shared_mutex> usernamesLock(thread->mutexUsernames);
			std::shared_lock<std::shared_mutex> filenameLock(thread->mutexFilename);
		}
		if (!thread->addSocket(soc, usernames[soc->socketDescriptor()])) {
			connectSlot(soc, connectReadyRead, connectDisconnected);
			return false;
		}
	}
	delete connectReadyRead;
	delete connectDisconnected;

	usernames.erase(soc->socketDescriptor());
	sockets.erase(soc->socketDescriptor());
	return true;
}

/**
 * This method reads the character, that was added from clients
 * @param soc
 * @return result of reading from socket
 */
bool Thread::readInsert(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readInsert()     ---------- READ INSERT ----------";
	if (soc == nullptr)
		return false;

	if (!readSpace(soc)) {
		return false;
	}
	int messageSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	QByteArray characterByteFormat;
	if (!readChunck(soc, characterByteFormat, messageSize)) {
		return false;
	}

	QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
	Character character = Character::toCharacter(jsonDocument);

	crdt->handleInsert(character);

	/* broadcast */
	if (!writeInsert(soc, character)) {
		return false;
	}

	needToSaveFile = true;
	if (!timerStarted) {
		saveTimer->start(saveInterval);
		timerStarted = true;
	}
	return true;
}

bool Thread::readStyleChanged(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readStyleChanged()     ---------- READ STYLE CHANGED ----------";
	if (soc == nullptr)
		return false;

	if (!readSpace(soc)) {
		return false;
	}
	int messageSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	QByteArray characterByteFormat;
	if (!readChunck(soc, characterByteFormat, messageSize)) {
		return false;
	}

	QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
	Character character = Character::toCharacter(jsonDocument);

	crdt->handleStyleChanged(character);

	/* broadcast */
	if (!writeStyleChanged(soc, character)) {
		return false;
	}

	needToSaveFile = true;
	if (!timerStarted) {
		saveTimer->start(saveInterval);
		timerStarted = true;
	}
	return true;
}

bool Thread::readAlignmentChanged(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readAlignmentChanged()     ---------- READ ALIGNMENT CHANGED ----------";

	//TODO: Manage better conversion from enum QByteArray
	readSpace(soc);
	int alignType = readNumberFromSocket(soc);
	readSpace(soc);

	int sizeBlockId = readNumberFromSocket(soc);

	readSpace(soc);

	QByteArray characterByteFormat;
	if (!readChunck(soc, characterByteFormat, sizeBlockId)) {
		return false;
	}

	QJsonDocument jsonFormatBlockId = QJsonDocument::fromBinaryData(characterByteFormat);
	Character blockId = Character::toCharacter(jsonFormatBlockId);

	int row = this->crdt->getRow(blockId);

	if (row < 0) return false;

	crdt->handleAlignmentChanged(alignType, row);

	if (!writeAlignmentChanged(soc, alignType, blockId)) {
		return false;
	}

	needToSaveFile = true;
	if (!timerStarted) {
		saveTimer->start(saveInterval);
		timerStarted = true;
	}
	return true;
}

/**
 * This method reads the character, that was removed from other users
 * @param soc
 * @return result of reading from socket
 */
bool Thread::readDelete(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readDelete()     ---------- READ DELETE ----------";
	if (soc == nullptr)
		return false;

	if (!readSpace(soc)) {
		return false;
	}
	int messageSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	QByteArray characterByteFormat;
	if (!readChunck(soc, characterByteFormat, messageSize)) {
		return false;
	}

	QJsonDocument jsonDocument = QJsonDocument::fromBinaryData(characterByteFormat);
	Character character = Character::toCharacter(jsonDocument);

	crdt->handleDelete(character);

	/* broadcast */
	if (!writeDelete(soc, character)) {
		return false;
	}

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
bool Thread::writeInsert(QTcpSocket *soc, Character &character) {
	qDebug() << "Thread.cpp - writeInsert()     ---------- WRITE INSERT ----------";
	if (soc == nullptr)
		return false;

	QByteArray message(INSERT_MESSAGE);
	QByteArray characterByteFormat = character.toQByteArray();
	QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

	message.append(" " + sizeOfMessage + " " + characterByteFormat);

	qDebug() << "                         " << message;
	qDebug() << ""; // newLine


	QByteArray username = usernames[soc->socketDescriptor()].toUtf8();
	QByteArray sizeOfSender = convertionNumber(username.size());
	message.append(" " + sizeOfSender + " " + username);
	qDebug() << "msg:" << " " + sizeOfSender + " " + username;

	//broadcast
	for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
		if (socket.first != soc->socketDescriptor()) {
			qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
			if (!writeMessage(socket.second, message)) {
				//return false;
			}
		}
	}
	return true;
}

bool Thread::writeStyleChanged(QTcpSocket *soc, Character &character) {
	qDebug() << "Thread.cpp - writeStyleChanged()     ---------- WRITE STYLE CHANGED ----------";
	if (soc == nullptr)
		return false;

	QByteArray message(STYLE_CAHNGED_MESSAGE);
	QByteArray characterByteFormat = character.toQByteArray();
	QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

	message.append(" " + sizeOfMessage + " " + characterByteFormat);

	QByteArray username = usernames[soc->socketDescriptor()].toUtf8();
	QByteArray sizeOfSender = convertionNumber(username.size());
	message.append(" " + sizeOfSender + " " + username);

	qDebug() << "msg:" << " " + sizeOfSender + " " + username;


	qDebug() << "                         " << message;
	qDebug() << ""; // newLine


	/* broadcast */
	for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
		if (socket.first != soc->socketDescriptor()) {
			if (!writeMessage(socket.second, message)) {
				//return false;
			}
		}
	}
	return true;
}

bool Thread::writeAlignmentChanged(QTcpSocket *soc, int alignment, Character &blockId) {
	qDebug() << "Thread.cpp - writeAlignmentChanged()     ---------- WRITE ALIGNMENT CHANGED ----------";
	if (soc == nullptr)
		return false;

	QByteArray message(ALIGNMENT_CHANGED_MESSAGE);
	QByteArray alignmentByteFormat = convertionNumber(alignment);
	QByteArray blockIdByteFormat = blockId.toQByteArray();
	//QByteArray sizeOfMessageAt = convertionNumber(alignmentByteFormat.size());
	QByteArray sizeBlockId = convertionNumber(blockIdByteFormat.size());

	message.append(" " + alignmentByteFormat);
	message.append(" " + sizeBlockId);
	message.append(" " + blockIdByteFormat);

	QByteArray username = usernames[soc->socketDescriptor()].toUtf8();
	QByteArray sizeOfSender = convertionNumber(username.size());

	message.append(" " + sizeOfSender + " " + username);

	qDebug() << "msg:" << " " + sizeOfSender + " " + username;
	qDebug() << "                         " << message;
	qDebug() << ""; // newLine

	/* broadcast */
	for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
		if (!writeMessage(socket.second, message)) {
			//return false;
		}
	}
	return true;
}

/**
 * This method sends the character, that was removed
 * @param character
 * @return result of writing on socket
 */
bool Thread::writeDelete(QTcpSocket *soc, Character &character) {
	qDebug() << "Thread.cpp - writeDelete()     ---------- WRITE DELETE ----------";
	if (soc == nullptr)
		return false;
	QByteArray message(DELETE_MESSAGE);
	QByteArray characterByteFormat = character.toQByteArray();
	QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

	message.append(" " + sizeOfMessage + " " + characterByteFormat);

	qDebug() << "                         " << message;
	qDebug() << ""; // newLine

	QByteArray username = usernames[soc->socketDescriptor()].toUtf8();
	QByteArray sizeOfSender = convertionNumber(username.size());

	message.append(" " + sizeOfSender + " " + username);
	qDebug() << "msg:" << " " + sizeOfSender + " " + username;

	/* broadcast */
	for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
		qDebug() << "userrname of user that send the delete message: " << usernames[soc->socketDescriptor()];
		if (socket.first != soc->socketDescriptor()) {
			qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
			if (!writeMessage(socket.second, message)) {
				//return false;
			}
		}
	}
	return true;
}

/**
 * This method sends the list of users, that are working on this file
 * @param soc
 */
bool Thread::sendListOfUsers(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - sendListOfUsers()     ---------- SEND LIST OF USER ----------";
	if (soc == nullptr)
		return false;

	QByteArray message(LIST_OF_USERS);
	if (usernames.size() - 1 == 0) {
		QByteArray usernamesSize = convertionNumber(0);
		message.append(usernamesSize);
	} else {
		QByteArray usernamesSize = convertionNumber(usernames.size() - 1 - pendingSocket.size());
		message.append(" " + usernamesSize);
		for (auto u : usernames) {
			if (u.first != soc->socketDescriptor() && pendingSocket.find(u.first) == pendingSocket.end()) {
				QByteArray usernameByteArray = convertionQString(u.second);
				QByteArray usernameSize = convertionNumber(usernameByteArray.size());

				message.append(" " + usernameSize + " " + usernameByteArray);
			}
		}
	}

	return writeMessage(soc, message);
}

/**
 * This method sends the new username, which has just been added
 * @param soc
 */
bool Thread::sendNewUser(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - sendNewUser()     ---------- SEND NEW USER ----------";
	if (soc == nullptr)
		return false;
	QByteArray message(LIST_OF_USERS);
	QByteArray usernamesSize = convertionNumber(1);
	QByteArray usernameByteArray = convertionQString(usernames[soc->socketDescriptor()]);
	QByteArray usernameSize = convertionNumber(usernameByteArray.size());
	message.append(" " + usernamesSize + " " + usernameSize + " " + usernameByteArray);

	for (auto s : sockets) {
		if (soc->socketDescriptor() != s.second->socketDescriptor()) {
			if (!writeMessage(s.second, message)) {
				return false;
			}
		}
	}
	return true;
}

/**
 * This method sends the username, which was disconnected
 * @param socketDescriptor
 * @param username
 */
bool Thread::sendRemoveUser(qintptr socketDescriptor, QString username) {
	/*if (sockets.size() == 1  && sockets.find(socketDescriptor) != sockets.end())
		return true;*/
	qDebug() << "sendRemoveUser";
	QByteArray message(REMOVE_USER);
	QByteArray usernameByteArray = convertionQString(username);
	QByteArray usernameSize = convertionNumber(usernameByteArray.size());
	message.append(" " + usernameSize + " " + usernameByteArray);

	for (auto s : sockets) {
		if (s.first != socketDescriptor) {
			qDebug() << usernames[s.first];
			if (!writeMessage(s.second, message)) {
				return false;
			}
		}
	}
	return true;
}

/**
 * This method sends all file to user
 * @param soc
 */
bool Thread::sendFile(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - sendFile()     ---------- SEND FILE ----------";
	if (soc == nullptr)
		return false;

	QByteArray message(SENDING_FILE);
	const std::vector<std::vector<Character>> file = crdt->getStructure();
	std::vector<std::pair<Character, int>> blockFormat = this->crdt->getStyle();
	QByteArray filenameByteArray = convertionQString(filename);
	QByteArray filenameSize = convertionNumber(filenameByteArray.size());
	QByteArray numLines = convertionNumber(file.size());

	message.append(" " + filenameSize + " " + filenameByteArray + " " + numLines);

	for (int i = 0; i < file.size(); i++) {
		std::vector<Character> line = file[i];
		QByteArray numChar = convertionNumber(file[i].size());

		message.append(" " + numChar);
		for (int j = 0; j < line.size(); j++) {
			Character character = line[j];
			QByteArray characterByteFormat = character.toQByteArray();
			QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

			message.append(" " + sizeOfMessage + " " + characterByteFormat);
		}
		QByteArray datas;
		if (!writeMessage(soc, message)) {
			return false;
		}
		if (!readChunck(soc, datas, 5)) {
			return false;
		}
		message.clear();
	}

	QByteArray numBlocks = convertionNumber(blockFormat.size());
	message.append(" " + numBlocks);

	for (std::pair<Character, int> &i : blockFormat) {
		Character character = i.first;
		QByteArray characterByteFormat = character.toQByteArray();
		QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

		message.append(" " + sizeOfMessage + " " + characterByteFormat);

		QByteArray alignment = convertionNumber(i.second);

		message.append(" " + alignment);
		QByteArray datas;
		if (!writeMessage(soc, message)) {
			return false;
		}
		if (!readChunck(soc, datas, 5)) {
			return false;
		}
		message.clear();
	}
	qDebug() << "                         " << message;
	qDebug() << "File inviato!"; // newLine

	/*if (!writeMessage(soc, message)) {
		return false;
	}*/
	return true;
}

/**
 * Read a shareCode from socket interact with DB
 * @param soc
 * @return bool
 */
bool Thread::readShareCode(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readShareCode()     ---------- READ SHARECODE ----------";
	if (soc == nullptr)
		return false;

	if (!readSpace(soc)) {
		return false;
	}

	/* shareCodeSize */
	int shareCodeSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	/* shareCode */
	QByteArray shareCode;
	if (!readChunck(soc, shareCode, shareCodeSize)) {
		return false;
	}

	QString filename;

	if (server->handleShareCode(usernames[soc->socketDescriptor()], shareCode, filename)) {
		return sendAddFile(soc, filename);
	} else {
		return false;
	}
}

/**
 * This method sends confirmation that the file has been shared.
 * @param soc
 * @param filename
 * @return bool
 */
bool Thread::sendAddFile(QTcpSocket *soc, QString filename) {
	qDebug() << "Thread.cpp - sendAddFile()     ---------- SEND ADD FILE ----------";
	if (soc == nullptr)
		return false;
	QByteArray message(ADD_FILE);
	QByteArray fileNameByteArray = convertionQString(filename);
	QByteArray fileNameSize = convertionNumber(fileNameByteArray.size());
	QByteArray shared;
	shared.setNum(0);
	message.append(" " + fileNameSize + " " + fileNameByteArray + " " + shared);

	return writeMessage(soc, message);
}

/**
 * This methods reads user's changes from client
 * @param soc
 * @return
 */
bool Thread::readEditAccount(QTcpSocket *soc) {
	qDebug() << "Server.cpp - readEditAccount()     ---------- READ EDIT ACCOUNT ----------";
	if (soc == nullptr)
		return false;

	if (!readSpace(soc)) {
		return false;
	}

	/* newUsernameSize */
	int newUsernameSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	/* newUsername */
	QString newUsername;
	if (!readQString(soc, newUsername, newUsernameSize)) {
		return false;
	}
	if (!readSpace(soc)) {
		return false;
	}

	/* newPasswordSize */
	int newPasswordSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	/* newPassword */
	QString newPassword;
	if (!readQString(soc, newPassword, newPasswordSize)) {
		return false;
	}
	if (!readSpace(soc)) {
		return false;
	}

	/* oldPasswordSize */
	int oldPasswordSize = readNumberFromSocket(soc);
	readSpace(soc);

	/* oldPassword */
	QString oldPassword;
	if (!readQString(soc, oldPassword, oldPasswordSize)) {
		return false;
	}
	if (!readSpace(soc)) {
		return false;
	}


	int sizeAvatar = readNumberFromSocket(soc);
	readSpace(soc);

	qDebug() << "                                username: " << newUsername << " size: " << newUsernameSize;
	qDebug() << "                                password: " << newPassword << " size: " << newPasswordSize;
	qDebug() << "                                password: " << oldPassword << " size: " << oldPasswordSize;
	qDebug() << "                                avatar size: " << sizeAvatar;

	/* avatar */
	QByteArray avatarDef;
	if (!readChunck(soc, avatarDef, sizeAvatar)) {
		return false;
	}

	qDebug() << "                                avatar size: " << sizeAvatar << " size read" << avatarDef.size();
	qDebug() << ""; // newLine

	Database db(QString::number((long long) QThread::currentThread(), 16));
	if (db.authenticateUser(usernames[soc->socketDescriptor()], oldPassword)) {
		if (newUsernameSize != 0) {
			if (db.changeUsername(usernames[soc->socketDescriptor()], newUsername)) {
				QDir dir("saveData/");
				dir.setNameFilters(QStringList(usernames[soc->socketDescriptor()] + "*"));
				dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
				std::map<qintptr, QTcpSocket *> serverSockets = this->server->getSockets();

				qDebug() << "Scanning: " << dir.path();

				QStringList fileList = dir.entryList();
				for (int i = 0; i < fileList.count(); i++) {
					if (fileList[i].split("%_##$$$##_%")[0] != usernames[soc->socketDescriptor()])
						continue;
					QString filename = fileList[i].split("%_##$$$##_%")[1].split(".json")[0];
					QString newFilename = newUsername + "%_##$$$##_%" + filename;
					QString oldFilename = fileList[i].split(".json")[0];
					auto thread = server->getThread(fileList[i].split(".json")[0]);
					server->changeNamethread(fileList[i].split(".json")[0], newFilename);

					qDebug() << "Found file: " << fileList[i];
					if (db.changeFileName(oldFilename, newFilename))
						renameFileSave(oldFilename, newFilename);
					else {
						qDebug() << "Unknown error renaming files";
						return false;
					}

					/* trovare tutti gli utenti che hanno in comune i file dell'utente */
					for (std::pair<qintptr, QString> username : this->server->getAllUsernames()) {
						if (username.first != soc->socketDescriptor()) {
							//std::map<QString, bool> listOfFile = this->server->getDb().getFiles(username.second);
							std::map<QString, bool> listOfFile = db.getFiles(username.second);

							auto result = listOfFile.find(newUsername + "%_##$$$##_%" + filename.split(".json")[0]);
							if (result != listOfFile.end()) {
								this->server->sendFileNames(serverSockets[username.first]);
							}
						}
					}
				}
				usernames[soc->socketDescriptor()] = newUsername;
				server->addUsername(soc->socketDescriptor(), newUsername);
			} else {
				qDebug() << "Err1";
				return false;
			}
		}

		if (newPasswordSize != 0) {
			if (db.changePassword(usernames[soc->socketDescriptor()], newPassword)) {
				qDebug() << "Err2";
				return false;
			}
		}

		if (sizeAvatar != 0) {
			if (db.changeAvatar(usernames[soc->socketDescriptor()], avatarDef)) {
				qDebug() << "Err3";
				return false;
			}
		}
	} else {
		qDebug() << "Err4";
		return false;
	}

	return true;
}

/**
 * This method sends the user's object
 * @param soc
 * @return result of writing on socket
 */
bool Thread::sendUser(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - sendUser()     ---------- SEND USER ----------";
	if (soc == nullptr) {
		return false;
	}
	QByteArray message(AVATAR_MESSAGE);
	QByteArray image;
	QString username;
	username = usernames[soc->socketDescriptor()];
	Database db(QString::number((long long) QThread::currentThread(), 16));
	image = db.getAvatar(username);
	QByteArray imageSize = convertionNumber(image.size());
	QByteArray usernameByteArray = convertionQString(username);
	QByteArray usernameSize = convertionNumber(usernameByteArray.size());

	message.append(" " + usernameSize + " " + usernameByteArray + " " + imageSize + " " + image);
//	qDebug() << message;

	return writeMessage(soc, message);
}

void Thread::disconnected(QTcpSocket *soc, qintptr socketDescriptor, QMetaObject::Connection *connectReadyRead,
						  QMetaObject::Connection *connectDisconnected) {
	std::unique_lock<std::shared_mutex> allUsernamesMutex(server->mutexAllUsernames);
	std::unique_lock<std::shared_mutex> serverSocketsMutex(server->mutexSockets);

	std::unique_lock<std::shared_mutex> socketsMutex(mutexSockets);
	std::unique_lock<std::shared_mutex> usernamesMutex(mutexUsernames);

	qDebug() << "Thread.cpp - disconnected()     " << socketDescriptor << " Disconnected";
	qDebug() << ""; // newLine
	disconnect(*connectReadyRead);
	disconnect(*connectDisconnected);
	delete connectReadyRead;
	delete connectDisconnected;
	QTcpSocket socket;
	socket.setSocketDescriptor(socketDescriptor);
	socket.deleteLater();
	if (sockets.find(socketDescriptor) != sockets.end()) {
		sockets.erase(socketDescriptor);
		sendRemoveUser(socketDescriptor, usernames[socketDescriptor]);
	} else {
		pendingSocket.erase(socketDescriptor);
	}
	server->removeSocket(socketDescriptor);
	usernames.erase(socketDescriptor);
	server->removeUsername(socketDescriptor);
	qDebug() << usernames;

	if (sockets.empty() && pendingSocket.empty()) {
		if (!fileDeleted)
			saveCRDTToFile();
		// dire al server di eliminare il thread dalla struttura
		QMetaObject::invokeMethod(server, "removeThread", Qt::QueuedConnection, Q_ARG(QString, filename));
	}
}

/**
 * This method reads the request for a list of usernames for a specific file
 * @param soc
 * @return
 */
bool Thread::readRequestUsernameList(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readFileName()     ---------- REQUEST USERNAME LIST ----------";
	if (soc == nullptr)
		return false;

	if (!readSpace(soc)) {
		return false;
	}

	/* fileNameSize */
	int fileNameSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	/* jsonFileName */
	QString jsonFileName;
	if (!readQString(soc, jsonFileName, fileNameSize)) {
		return false;
	}

	qDebug() << "                               " << jsonFileName;
	Database db(QString::number((long long) QThread::currentThread(), 16));
	QStringList userlist = db.getUsers(jsonFileName);
	qDebug() << userlist;


	int nFiles = userlist.size();
	QByteArray message(USERNAME_LIST_FOR_FILE);
	QByteArray numUsers = convertionNumber(nFiles);

	message.append(" " + numUsers);

	for (QString username : userlist) {
		QByteArray usernameQBytearray = convertionQString(username);
		QByteArray usernameSize = convertionNumber(usernameQBytearray.size());
		message.append(" " + usernameSize + " " + usernameQBytearray);
	}

	return writeMessage(soc, message);
}

/**
 * This method reads the file information changes
 * @param soc
 * @return
 */
bool Thread::readFileInformationChanges(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readDeleteFile()     ---------- READ DELETE FILE ----------";
	if (soc == nullptr)
		return false;

	if (!readSpace(soc)) {
		return false;
	}

	/* oldFileNameSize */
	int oldFileNameSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	/* oldJsonFileName */
	QString oldJsonFileName;
	if (!readQString(soc, oldJsonFileName, oldFileNameSize)) {
		return false;
	}
	if (!readSpace(soc)) {
		return false;
	}

	/* newFileNameSize */
	int newFileNameSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	/* newJsonFileName */
	QString newJsonFileName;
	if (!readQString(soc, newJsonFileName, newFileNameSize)) {
		return false;
	}
	if (!readSpace(soc)) {
		return false;
	}

	/* usernamesSize */
	int usernamesSize = readNumberFromSocket(soc);
	QStringList removedUsers;

	qDebug() << usernamesSize;

	Database db(QString::number((long long) QThread::currentThread(), 16));

	if (usernamesSize != 0) {
		for (int i = 0; i < usernamesSize; i++) {
			readSpace(soc);
			int usernameSize = readNumberFromSocket(soc);
			qDebug() << usernameSize;
			readSpace(soc);
			QString username;
			if (!readQString(soc, username, usernameSize)) {
				return false;
			}
			qDebug() << "                              usename: " << username;
			db.removePermission(oldJsonFileName, username);
			removedUsers.append(username);
		}
	}

	qDebug() << oldJsonFileName << newJsonFileName;
	std::map<qintptr, QTcpSocket *> serverSockets = this->server->getSockets();
	std::map<qintptr, QString> serverAllUsernames = this->server->getAllUsernames();

	if (newFileNameSize != 0 && newJsonFileName != oldJsonFileName) {
		if (db.changeFileName(oldJsonFileName, newJsonFileName)) {
			renameFileSave(oldJsonFileName, newJsonFileName);
			server->changeNamethread(oldJsonFileName, newJsonFileName);
		} else return false;
		/* server*/
		for (std::pair<qintptr, QString> username : serverAllUsernames) {
			if (username.first != soc->socketDescriptor()) {
				std::map<QString, bool> listOfFile = db.getFiles(username.second);

				auto result = listOfFile.find(newJsonFileName);
				if (result != listOfFile.end()) {
					if (!this->server->sendFileNames(serverSockets[username.first])) {
						return false;
					}
				}
			}
		}
	}

	/* server */
	for (QString removedUsername : removedUsers) {
		for (std::pair<qintptr, QString> username : serverAllUsernames) {
			if (username.first != soc->socketDescriptor() || removedUsername == username.second) {
				if (!this->server->sendFileNames(serverSockets[username.first])) {
					return false;
				}
			}
		}
	}

	/* thread */
	for (QString removedUsername : removedUsers) {
		for (std::pair<qintptr, QString> username : usernames) {
			if (removedUsername == username.second) {
				sendRemoveUser(username.first, username.second);
				addPendingSocket(username.first);
				break;
			}
		}
	}

	return this->server->sendFileNames(soc);
}

/**
 * This method handles the elimination of a specific file
 * @param soc
 * @return
 */
bool Thread::readDeleteFile(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readDeleteFile()     ---------- READ DELETE FILE ----------";
	if (soc == nullptr)
		return false;

	if (!readSpace(soc)) {
		return false;
	}

	/* fileNameSize */
	int fileNameSize = readNumberFromSocket(soc);
	if (!readSpace(soc)) {
		return false;
	}

	/* jsonFileName */
	QString jsonFileName;
	if (!readQString(soc, jsonFileName, fileNameSize)) {
		return false;
	}

	Database db(QString::number((long long) QThread::currentThread(), 16));
	qDebug() << "                               " << jsonFileName;
	std::map<QString, qintptr> usersremove;
	for (std::pair<qintptr, QString> username : server->getAllUsernames()) {
		if (username.first != soc->socketDescriptor()) {
			//std::map<QString, bool> listOfFile = server->getDb().getFiles(username.second);
			std::map<QString, bool> listOfFile = db.getFiles(username.second);

			auto result = listOfFile.find(jsonFileName);
			if (result != listOfFile.end()) {
				qDebug() << username.second;
				usersremove[username.second] = username.first;
			}
		}
	}
	if (!db.deleteFile(jsonFileName)) {
		return false;
	}

	needToSaveFile = false;
	if (!deleteFileSave(jsonFileName))
		return false;

	auto thread = server->getThread(jsonFileName);
	if (thread != nullptr) {
		if (thread.get() != this) {
			std::unique_lock<std::shared_mutex> threadSocketsMutex(thread->mutexSockets);
			std::shared_lock<std::shared_mutex> usernamesMutex(thread->mutexUsernames);
			std::unique_lock<std::shared_mutex> pendingSocketsMutex(thread->mutexPendingSockets);
			std::unique_lock<std::shared_mutex> needToSaveMutex(thread->mutexNeedToSave);
			std::unique_lock<std::shared_mutex> fileDeletedMutex(thread->mutexFileDeleted);
		}
		auto sockets = thread->getSockets();

		for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
			server->sendFileNames(socket.second);
		}

		thread->deleteFile();
	}

	server->addDeleteFileThread(jsonFileName);

	std::map<qintptr, QTcpSocket *> serverSockets = server->getSockets();

	for (std::pair<QString, qintptr> user : usersremove) {
		if (!this->server->sendFileNames(serverSockets[user.second])) {
			return false;
		}
	}

	return true;
}

std::map<qintptr, QTcpSocket *> Thread::getSockets() {
	return sockets;
}

const std::map<qintptr, QString> &Thread::getUsernames() const {
	return usernames;
}

void Thread::changeFileName(QString filename) {
	this->filename = filename;
}

/**
 * This method adds the socket inside the pendingSocket structure
 * @param socketDescriptor
 */
void Thread::addPendingSocket(qintptr socketDescriptor) {
	pendingSocket[socketDescriptor] = sockets[socketDescriptor];
	sockets.erase(socketDescriptor);
}

/**
 * this method handles the elimiantion of the thread
 */
void Thread::deleteFile() {
	// Gestire la concorrenza
	for (std::pair<qintptr, QString> user: usernames) {
		QByteArray message(REMOVE_USER);
		QByteArray usernameByteArray = convertionQString(user.second);
		QByteArray usernameSize = convertionNumber(usernameByteArray.size());
		message.append(" " + usernameSize + " " + usernameByteArray);

		if (!writeMessage(sockets[user.first], message)) {
			return;
		}
		pendingSocket[user.first] = sockets[user.first];
		sockets.erase(user.first);
	}
	needToSaveFile = false;
	fileDeleted = true;
}

/**
 * This function is used for renaming all saved files (including backup ones)
 * @return true if renamed, false in case of error
 */
bool Thread::renameFileSave(QString oldFilename, QString newFilename) {
	bool result = true;
	QString directories[] = {"saveData", "backup1", "backup2"};

	for (QString dir : directories) {
		QFile savefile(dir + "/" + oldFilename + ".json");
		if (savefile.exists()) {
			if (!savefile.rename(dir + "/" + newFilename + ".json")) {
				result = false;
				qDebug() << "Error renaming: " << savefile.fileName();
			} else {
				qDebug() << "Renamed '" + oldFilename + "' into '" + newFilename + "'";
			}
		}
	}
	return result;
}

/**
 * This function delete a file from the server and also from all the backup directories
 * @param filename
 * @return true if deleted, false in case of error
 */
bool Thread::deleteFileSave(QString filename) {
	bool result = true;
	QString directories[] = {"saveData", "backup1", "backup2"};

	for (QString dir : directories) {
		QFile savefile(dir + "/" + filename + ".json");
		if (savefile.exists()) {
			if (!savefile.remove()) {
				result = false;
				qDebug() << "Error deleting: " << savefile.fileName();
			} else {
				qDebug() << "Deleted: " + filename;
			}
		}
	}
	return result;
}
