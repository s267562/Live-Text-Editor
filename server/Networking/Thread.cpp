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
void Thread::addSocket(QTcpSocket *soc, QString username) {
	qintptr socketDescriptor = soc->socketDescriptor();
	/* insert new socket into structure */
	sockets[socketDescriptor] = soc;
	//sockets[socketDescriptor] = new QTcpSocket();
	//sockets[socketDescriptor]->setSocketDescriptor(socketDescriptor);
	sockets[socketDescriptor]->setParent(nullptr);
	sockets[socketDescriptor]->moveToThread(this);
	usernames[socketDescriptor] = username;
	qDebug() << "Thread.cpp - addSocket()     sockets.size" << sockets.size();

	QMetaObject::Connection *connectReadyRead = new QMetaObject::Connection();
	QMetaObject::Connection *connectDisconnected = new QMetaObject::Connection();

	/* connect socket and signal */
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

	pendingSocket.erase(
			socketDescriptor);      //tolgo dalla lista l'utente che ha di nuovo il permesso di modificare il file
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
void Thread::readyRead(QTcpSocket *soc, QMetaObject::Connection *connectReadyRead,
					   QMetaObject::Connection *connectDisconnected) {
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
	std::shared_lock<std::shared_mutex> fileDeletedMutex(mutexFileDeleted);
	if (data.toStdString() == INSERT_MESSAGE && !fileDeleted) {
		std::unique_lock<std::shared_mutex> needToSaveMutex(mutexNeedToSave);
		if (!readInsert(soc)) {
			writeErrMessage(soc, INSERT_MESSAGE);
		}
		writeOkMessage(soc);
		readyRead(soc, connectReadyRead, connectDisconnected);
	} else if (data.toStdString() == STYLE_CAHNGED_MESSAGE && !fileDeleted) {
		std::unique_lock<std::shared_mutex> needToSaveMutex(mutexNeedToSave);
		if (!readStyleChanged(soc)) {
			writeErrMessage(soc);
		}
		writeOkMessage(soc);
		readyRead(soc, connectReadyRead, connectDisconnected);
	} else if (data.toStdString() == DELETE_MESSAGE && !fileDeleted) {
		std::unique_lock<std::shared_mutex> needToSaveMutex(mutexNeedToSave);
		if (!readDelete(soc)) {
			writeErrMessage(soc, DELETE_MESSAGE);
		}
		needToSaveMutex.unlock();
		readyRead(soc, connectReadyRead, connectDisconnected);
	} else if (data.toStdString() == REQUEST_FILE_MESSAGE) {
		std::unique_lock<std::shared_mutex> socketsLock(mutexSockets);
		std::unique_lock<std::shared_mutex> pendingSocketsLock(mutexPendingSockets);
		std::unique_lock<std::shared_mutex> usernamesLock(mutexUsernames);
		std::shared_lock<std::shared_mutex> filenameLock(mutexFilename);

		std::unique_lock<std::shared_mutex> threadsMutex(server->mutexThread);
		readSpace(soc);
		int fileNameSize = readNumberFromSocket(soc);
		readSpace(soc);

		QString jsonFileName;

		if (!readQString(soc, jsonFileName, fileNameSize)) {
			writeErrMessage(soc, REQUEST_FILE_MESSAGE);
			return;
		}

		if (jsonFileName == this->filename && !fileDeleted) {
			if (pendingSocket.find(soc->socketDescriptor()) != pendingSocket.end()) {
				sockets[soc->socketDescriptor()] = soc;
				pendingSocket.erase(soc->socketDescriptor());
				sendFile(soc);
				sendListOfUsers(soc);
				sendNewUser(soc);
			} else {
				sendListOfUsers(soc);
			}
			return;
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
		delete connectReadyRead;
		delete connectDisconnected;
		std::shared_ptr<Thread> thread = server->getThread(jsonFileName);

		sendRemoveUser(soc->socketDescriptor(), usernames[soc->socketDescriptor()]);
		if (thread.get() == nullptr) {
			/* thread doesn't exist */
			thread = server->addThread(jsonFileName, usernames[soc->socketDescriptor()]);
			thread->addSocket(soc, usernames[soc->socketDescriptor()]);
			thread->start();
		} else {
			if (thread.get() != this) {
				std::unique_lock<std::shared_mutex> socketsLock(thread->mutexSockets);
				std::unique_lock<std::shared_mutex> pendingSocketsLock(thread->mutexPendingSockets);
				std::unique_lock<std::shared_mutex> usernamesLock(thread->mutexUsernames);
				std::shared_lock<std::shared_mutex> filenameLock(thread->mutexFilename);
			}
			thread->addSocket(soc, usernames[soc->socketDescriptor()]);
		}

		qDebug() << soc->socketDescriptor() << " " << usernames[soc->socketDescriptor()];
		usernames.erase(soc->socketDescriptor());
		sockets.erase(soc->socketDescriptor());
	} else if (data.toStdString() == SHARE_CODE) {
		std::shared_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
		if (!readShareCode(soc)) {
			writeErrMessage(soc, DELETE_MESSAGE);
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
		std::unique_lock<std::shared_mutex> threadsMutex(server->mutexUsernames);
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
	std::shared_lock<std::shared_mutex> filenameLock(mutexFilename);
	std::unique_lock<std::shared_mutex> needToSaveLock(mutexNeedToSave);

	QString jsonFileName = /*usernameOwner + "%_##$$$##_%" +*/ filename;
	if (needToSaveFile) {
		qDebug() << "Saving CRDT for file: " + jsonFileName;
		crdt->saveCRDT(jsonFileName);
		needToSaveFile = false;
	}
}

/**
 * This method reads the character, that was added from clients
 * @param soc
 * @return result of reading from socket
 */
bool Thread::readInsert(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readInsert()     ---------- READ INSERT ----------";

	readSpace(soc);
	int messageSize = readNumberFromSocket(soc);
	readSpace(soc);

	QByteArray characterByteFormat;
	if (!readChunck(soc, characterByteFormat, messageSize)) {
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

bool Thread::readStyleChanged(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readStyleChanged()     ---------- READ STYLE CHANGED ----------";

	readSpace(soc);
	int messageSize = readNumberFromSocket(soc);
	readSpace(soc);

	QByteArray characterByteFormat;
	if (!readChunck(soc, characterByteFormat, messageSize)) {
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

/**
 * This method reads the character, that was removed from other users
 * @param soc
 * @return result of reading from socket
 */
bool Thread::readDelete(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readDelete()     ---------- READ DELETE ----------";

	readSpace(soc);
	int messageSize = readNumberFromSocket(soc);
	readSpace(soc);

	QByteArray characterByteFormat;
	if (!readChunck(soc, characterByteFormat, messageSize)) {
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
void Thread::writeInsert(QTcpSocket *soc, Character character) {
	qDebug() << "Thread.cpp - writeInsert()     ---------- WRITE INSERT ----------";

	QByteArray message(INSERT_MESSAGE);
	QByteArray characterByteFormat = character.toQByteArray();
	QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

	message.append(" " + sizeOfMessage + " " + characterByteFormat);

	qDebug() << "                         " << message;
	qDebug() << ""; // newLine

	//broadcast
	for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
		if (socket.first != soc->socketDescriptor()) {
			//qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
			writeMessage(socket.second, message);
		}
	}
}

void Thread::writeStyleChanged(QTcpSocket *soc, Character character) {
	qDebug() << "Thread.cpp - writeStyleChanged()     ---------- WRITE STYLE CHANGED ----------";

	QByteArray message(STYLE_CAHNGED_MESSAGE);
	QByteArray characterByteFormat = character.toQByteArray();
	QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

	message.append(" " + sizeOfMessage + " " + characterByteFormat);

	qDebug() << "                         " << message;
	qDebug() << ""; // newLine

	//broadcast
	for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
		// qDebug() << "userrname of user that send the delete message: " << usernames[soc->socketDescriptor()];
		if (socket.first != soc->socketDescriptor()) {
			//qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
			writeMessage(socket.second, message);
		}
	}
}

/**
 * This method sends the character, that was removed
 * @param character
 * @return result of writing on socket
 */
void Thread::writeDelete(QTcpSocket *soc, Character character) {
	qDebug() << "Thread.cpp - writeDelete()     ---------- WRITE DELETE ----------";

	QByteArray message(DELETE_MESSAGE);
	QByteArray characterByteFormat = character.toQByteArray();
	QByteArray sizeOfMessage = convertionNumber(characterByteFormat.size());

	message.append(" " + sizeOfMessage + " " + characterByteFormat);

	qDebug() << "                         " << message;
	qDebug() << ""; // newLine

	//broadcast
	for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
		// qDebug() << "userrname of user that send the delete message: " << usernames[soc->socketDescriptor()];
		if (socket.first != soc->socketDescriptor()) {
			//qDebug() << "Sending to:" << usernames[socket.second->socketDescriptor()];
			writeMessage(socket.second, message);
		}
	}
}

/**
 * This method sends the list of users, that are working on this file
 * @param soc
 */
void Thread::sendListOfUsers(QTcpSocket *soc) {
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

	if (!writeMessage(soc, message)) {
		return;
	}
}

/**
 * This method sends the new username, which has just been added
 * @param soc
 */
void Thread::sendNewUser(QTcpSocket *soc) {
	QByteArray message(LIST_OF_USERS);
	QByteArray usernamesSize = convertionNumber(1);
	QByteArray usernameByteArray = convertionQString(usernames[soc->socketDescriptor()]);
	QByteArray usernameSize = convertionNumber(usernameByteArray.size());
	message.append(" " + usernamesSize + " " + usernameSize + " " + usernameByteArray);

	for (auto s : sockets) {
		if (soc->socketDescriptor() != s.second->socketDescriptor()) {
			if (!writeMessage(s.second, message)) {
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
void Thread::sendRemoveUser(qintptr socketDescriptor, QString username) {
	qDebug() << "sendRemoveUser";
	QByteArray message(REMOVE_USER);
	QByteArray usernameByteArray = convertionQString(username);
	QByteArray usernameSize = convertionNumber(usernameByteArray.size());
	message.append(" " + usernameSize + " " + usernameByteArray);

	for (auto s : sockets) {
		//if (socketDescriptor != s.first) {
		qDebug() << usernames[s.first];
		if (!writeMessage(s.second, message)) {
			return;
		}
		//}
	}
}

/**
 * This method sends all file to user
 * @param soc
 */
void Thread::sendFile(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - sendFile()     ---------- SEND FILE ----------";
	QByteArray message(SENDING_FILE);
	const std::vector<std::vector<Character>> file = crdt->getStructure();
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
	}
	qDebug() << "                         " << message;
	qDebug() << ""; // newLine

	if (!writeMessage(soc, message)) {
		return;
	}
}

bool Thread::readShareCode(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readShareCode()     ---------- READ SHARECODE ----------";
	readSpace(soc);
	int shareCodeSize = readNumberFromSocket(soc);
	readSpace(soc);

	QByteArray shareCode;
	if (!readChunck(soc, shareCode, shareCodeSize)) {
		return false;
	}

	qDebug() << shareCodeSize << shareCode;
	QString filename;

	if (server->handleShareCode(usernames[soc->socketDescriptor()], shareCode, filename)) {
		sendAddFile(soc, filename);
		return true;
	} else {
		writeErrMessage(soc, SHARE_CODE);
		return false;
	}
}

bool Thread::sendAddFile(QTcpSocket *soc, QString filename) {
	QByteArray message(ADD_FILE);

	QByteArray fileNameByteArray = convertionQString(filename);
	QByteArray fileNameSize = convertionNumber(fileNameByteArray.size());
	QByteArray shared;
	shared.setNum(0);
	message.append(" " + fileNameSize + " " + fileNameByteArray + " " + shared);

	if (!writeMessage(soc, message)) {
		return false;
	}

	return true;
}

bool Thread::readEditAccount(QTcpSocket *soc) {
	qDebug() << "Server.cpp - readEditAccount()     ---------- READ EDIT ACCOUNT ----------";
	if (soc == nullptr) {
		return false;
	}
	readSpace(soc);
	int newUsernameSize = readNumberFromSocket(soc);
	readSpace(soc);

	//username
	QString newUsername;
	if (!readQString(soc, newUsername, newUsernameSize)) {
		return false;
	}
	readSpace(soc);

	int newPasswordSize = readNumberFromSocket(soc);
	readSpace(soc);

	//password
	QString newPassword;
	if (!readQString(soc, newPassword, newPasswordSize)) {
		return false;
	}
	readSpace(soc);

	int oldPasswordSize = readNumberFromSocket(soc);
	readSpace(soc);

	//password
	QString oldPassword;
	if (!readQString(soc, oldPassword, oldPasswordSize)) {
		return false;
	}
	readSpace(soc);


	int sizeAvatar = readNumberFromSocket(soc);
	readSpace(soc);

	qDebug() << "                                username: " << newUsername << " size: " << newUsernameSize;
	qDebug() << "                                password: " << newPassword << " size: " << newPasswordSize;
	qDebug() << "                                password: " << oldPassword << " size: " << oldPasswordSize;
	qDebug() << "                                avatar size: " << sizeAvatar;

	//avatar
	QByteArray avatarDef;

	if (!readChunck(soc, avatarDef, sizeAvatar)) {
		return false;
	}

	qDebug() << "                                avatar size: " << sizeAvatar << " size read" << avatarDef.size();

	qDebug() << ""; // newLine

	Database db(QString::number((long long) QThread::currentThread(), 16));
	//if (server->getDb().authenticateUser(usernames[soc->socketDescriptor()], oldPassword)) {
	if (db.authenticateUser(usernames[soc->socketDescriptor()], oldPassword)) {
		if (newUsernameSize != 0) {
			//if (server->getDb().changeUsername(usernames[soc->socketDescriptor()], newUsername)) {
			if (db.changeUsername(usernames[soc->socketDescriptor()], newUsername)) {
				QDir dir;
				dir.setNameFilters(QStringList(usernames[soc->socketDescriptor()] + "*"));
				dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
				std::map<qintptr, QTcpSocket *> serverSockets = this->server->getSockets();

				qDebug() << "Scanning: " << dir.path();

				QStringList fileList = dir.entryList();
				for (int i = 0; i < fileList.count(); i++) {
					QString filename = fileList[i].split("%_##$$$##_%")[1].split(".json")[0];
					QString newFilename = newUsername + "%_##$$$##_%" + filename;
					auto thread = server->getThread(fileList[i].split(".json")[0]);
					//if (thread != nullptr) {
					//std::unique_lock<std::shared_mutex> filenameLock(thread->mutexFilename);
					//thread->changeFileName(newFilename);
					server->changeNamethread(fileList[i].split(".json")[0], newFilename);

					//}
					qDebug() << "Found file: " << fileList[i];
					QFile renamefile(fileList[i]);

					renamefile.rename(newFilename + ".json");
					//server->getDb().changeFileName(fileList[i].split(".json")[0], newFilename);
					db.changeFileName(fileList[i].split(".json")[0], newFilename);
					renamefile.close();


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
			//if (server->getDb().changePassword(usernames[soc->socketDescriptor()], newPassword)) {
			if (db.changePassword(usernames[soc->socketDescriptor()], newPassword)) {
				qDebug() << "Err2";
				return false;
			}
		}

		if (sizeAvatar != 0) {
			//if (!server->getDb().changeAvatar(usernames[soc->socketDescriptor()], avatarDef)) {
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

bool Thread::sendUser(QTcpSocket *soc) {
	qDebug() << "Server.cpp - sendUser()     ---------- SEND USER ----------";
	QByteArray message(AVATAR_MESSAGE);
	QByteArray image;
	//image = "image";
	QString username;
	username = usernames[soc->socketDescriptor()];
	Database db(QString::number((long long) QThread::currentThread(), 16));
	//image = server->getDb().getAvatar(username);
	image = db.getAvatar(username);
	QByteArray imageSize = convertionNumber(image.size());
	QByteArray usernameByteArray = convertionQString(username);
	QByteArray usernameSize = convertionNumber(usernameByteArray.size());

	message.append(" " + usernameSize + " " + usernameByteArray + " " + imageSize + " " + image);

	qDebug() << message;

	if (!writeMessage(soc, message)) {
		return false;
	}

	return true;
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

	if (sockets.empty()) {
		//this->setParent(nullptr);
		//this->deleteLater();
		//this->quit();
		//this->terminate();
	}
}

bool Thread::readRequestUsernameList(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readFileName()     ---------- REQUEST USERNAME LIST ----------";
	readSpace(soc);
	int fileNameSize = readNumberFromSocket(soc);
	readSpace(soc);

	QString jsonFileName;
	if (!readQString(soc, jsonFileName, fileNameSize)) {
		return false;
	}

	qDebug() << "                               " << jsonFileName;
	Database db(QString::number((long long) QThread::currentThread(), 16));
	//QStringList userlist = this->server->getDb().getUsers(jsonFileName);
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

bool Thread::readFileInformationChanges(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readDeleteFile()     ---------- READ DELETE FILE ----------";
	readSpace(soc);
	int oldFileNameSize = readNumberFromSocket(soc);
	readSpace(soc);

	QString oldJsonFileName;
	if (!readQString(soc, oldJsonFileName, oldFileNameSize)) {
		return false;
	}

	readSpace(soc);
	int newFileNameSize = readNumberFromSocket(soc);
	readSpace(soc);

	QString newJsonFileName;
	if (!readQString(soc, newJsonFileName, newFileNameSize)) {
		return false;
	}

	readSpace(soc);
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
			//this->server->getDb().removePermission(oldJsonFileName, username);
			db.removePermission(oldJsonFileName, username);
			removedUsers.append(username);
		}
	}

	qDebug() << oldJsonFileName << newJsonFileName;
	std::map<qintptr, QTcpSocket *> serverSockets = this->server->getSockets();
	std::map<qintptr, QString> serverAllUsernames = this->server->getAllUsernames();

	if (newFileNameSize != 0 && newJsonFileName != oldJsonFileName) {
		QFile saveFile(oldJsonFileName + ".json");
		//this->server->getDb().changeFileName(oldJsonFileName, newJsonFileName);
		db.changeFileName(oldJsonFileName, newJsonFileName);
		saveFile.rename(newJsonFileName + ".json");
		saveFile.close();
		server->changeNamethread(oldJsonFileName, newJsonFileName);

		/* server*/
		for (std::pair<qintptr, QString> username : serverAllUsernames) {
			if (username.first != soc->socketDescriptor()) {
				//std::map<QString, bool> listOfFile = this->server->getDb().getFiles(username.second);
				std::map<QString, bool> listOfFile = db.getFiles(username.second);

				auto result = listOfFile.find(newJsonFileName);
				if (result != listOfFile.end()) {
					this->server->sendFileNames(serverSockets[username.first]);
				}
			}
		}
	}

	/* server */
	for (QString removedUsername : removedUsers) {
		for (std::pair<qintptr, QString> username : serverAllUsernames) {
			if (username.first != soc->socketDescriptor() || removedUsername == username.second) {
				this->server->sendFileNames(serverSockets[username.first]);
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


	this->server->sendFileNames(soc);
	return true;
}

bool Thread::readDeleteFile(QTcpSocket *soc) {
	qDebug() << "Thread.cpp - readDeleteFile()     ---------- READ DELETE FILE ----------";
	readSpace(soc);
	int fileNameSize = readNumberFromSocket(soc);
	readSpace(soc);

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
	//if (!this->server->getDb().deleteFile(jsonFileName)) {
	if (!db.deleteFile(jsonFileName)) {
		return false;
	}

	needToSaveFile = false;
	QFile deletefile(jsonFileName + ".json");
	deletefile.remove();
	deletefile.close();

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

void Thread::addPendingSocket(qintptr socketDescriptor) {
	pendingSocket[socketDescriptor] = sockets[socketDescriptor];
	sockets.erase(socketDescriptor);
}

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