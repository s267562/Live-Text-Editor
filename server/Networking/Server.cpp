#include <QDataStream>
#include <shared_mutex>
#include "Server.h"
#include "../SimpleCrypt/SimpleCrypt.h"

Server::Server(QObject *parent) : QTcpServer(parent) {}

/**
 * This method starts server's listening on specific port
 * @param port
 * @return
 */
bool Server::startServer(quint16 port) {
	connect(this, SIGNAL(newConnection()), this, SLOT(connection()));
	if (!this->listen(QHostAddress::Any, port)) {
		qDebug() << "Server.cpp - startServer()     Could not start server";
		qDebug() << ""; // newLine
		return false;
	} else {
		qDebug() << "Server.cpp - startServer()     Listening to port " << port << "...";
		qDebug() << ""; // newLine
		return true;
	}

}

/**
 *
 */
void Server::connection() {
    std::unique_lock<std::shared_mutex> socketsMutex(mutexSockets);
    QTcpSocket *soc = this->nextPendingConnection();

	qintptr socketDescriptor = soc->socketDescriptor();
	socketsState[socketDescriptor] = UNLOGGED;
	sockets[socketDescriptor] = soc;
	QMetaObject::Connection *connectReadyRead = new QMetaObject::Connection();
	QMetaObject::Connection *connectDisconnected = new QMetaObject::Connection();
	soc->setParent(nullptr);

	*connectReadyRead = connect(soc, &QTcpSocket::readyRead, this,
								[this, connectReadyRead, connectDisconnected, soc, socketDescriptor] {
									readyRead(connectReadyRead, connectDisconnected, soc, socketDescriptor);
								}, Qt::QueuedConnection);

	*connectDisconnected = connect(soc, &QTcpSocket::disconnected, this,
								   [this, connectReadyRead, connectDisconnected, soc, socketDescriptor] {
									   disconnected(connectReadyRead, connectDisconnected, soc, socketDescriptor);
								   });
}

/**
 * This method allows to start reading from socket and calls different methods
 * that handles different type of message.
 */
void Server::readyRead(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *connectDisconnected,
					   QTcpSocket *soc, qintptr socketDescriptor) {
	QByteArray data;
	if (!readChunck(soc, data, 5)) {
		writeErrMessage(soc);
		soc->flush();
		return;
	}

	qDebug() << "Server.cpp - connection()     msg received:" << data;

	if (data.toStdString() == LOGIN_MESSAGE && socketsState[socketDescriptor] == UNLOGGED) {
	    std::unique_lock<std::shared_mutex> allUsernamesMutex(mutexAllUsernames);
        std::unique_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
        if (logIn(soc)) {
			sendUser(soc);
			sendFileNames(soc);
			socketsState[socketDescriptor] = LOGGED;
			qDebug() << "                              socketsSize: " << socketsState.size();
			qDebug() << ""; // newLine
		} else {
			//error in login phase
			qDebug() << "                              error login";
			qDebug() << ""; // newLine
			writeErrMessage(soc, LOGIN_MESSAGE);
			return;
		}
	} else if (data.toStdString() == REGISTRATION_MESSAGE && socketsState[socketDescriptor] == UNLOGGED) {
        std::unique_lock<std::shared_mutex> allUsernamesMutex(mutexAllUsernames);
        std::unique_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
	    if (registration(soc)) {
			sendUser(soc);
			sendFileNames(soc);
			socketsState[socketDescriptor] = LOGGED;
		} else {
			writeErrMessage(soc, REGISTRATION_MESSAGE);
			return;
		}
	} else if (data.toStdString() == REQUEST_FILE_MESSAGE && socketsState[socketDescriptor] == LOGGED) {
        std::unique_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
	    std::unique_lock<std::shared_mutex> threadsMutex(mutexThread);
	    if (readFileName(socketDescriptor, soc)) {
			/* disconnect from main thread */
			disconnect(*connectReadyRead);
			disconnect(*connectDisconnected);
			delete connectReadyRead;
			delete connectDisconnected;
			socketsState.erase(socketDescriptor);
			qDebug() << "                              socketsSize: " << socketsState.size();
			qDebug() << ""; // newLine
		} else {
			writeErrMessage(soc, REQUEST_FILE_MESSAGE);
		}
	} else if (data.toStdString() == EDIT_ACCOUNT && socketsState[socketDescriptor] == LOGGED) {
        std::unique_lock<std::shared_mutex> allUsernamesMutex(mutexAllUsernames);
        std::unique_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
        std::unique_lock<std::shared_mutex> threadsMutex(mutexThread);
        std::unique_lock<std::shared_mutex> socketsMutex(mutexSockets);
        if (readEditAccount(soc)) {
			sendUser(soc);
			sendFileNames(soc);
		} else {
			writeErrMessage(soc, EDIT_ACCOUNT);
		}
	} else if (data.toStdString() == SHARE_CODE && socketsState[socketDescriptor] == LOGGED) {
        std::shared_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
        if (!readShareCode(soc)) {
			writeErrMessage(soc, SHARE_CODE);
		}
	} else if (data.toStdString() == REQUEST_USERNAME_LIST_MESSAGE && socketsState[socketDescriptor] == LOGGED){
        if (!readRequestUsernameList(soc)) {
            writeErrMessage(soc, REQUEST_USERNAME_LIST_MESSAGE);
        }
	}  else if (data.toStdString() == FILE_INFORMATION_CHANGES && socketsState[socketDescriptor] == LOGGED){
        std::shared_lock<std::shared_mutex> allUsernamesMutex(mutexAllUsernames);
        std::shared_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
        std::unique_lock<std::shared_mutex> threadsMutex(mutexThread);
        std::unique_lock<std::shared_mutex> socketsMutex(mutexSockets);
        if (!readFileInformationChanges(soc)) {
            writeErrMessage(soc, FILE_INFORMATION_CHANGES);
        }
    } else if (data.toStdString() == DELETE_FILE && socketsState[socketDescriptor] == LOGGED){
        std::shared_lock<std::shared_mutex> allUsernamesMutex(mutexAllUsernames);
        std::shared_lock<std::shared_mutex> usernamesMutex(mutexUsernames);
        std::unique_lock<std::shared_mutex> threadsMutex(mutexUsernames);
        std::unique_lock<std::shared_mutex> socketsMutex(mutexSockets);
	    if (!readDeleteFile(soc)) {
            writeErrMessage(soc, DELETE_FILE);
        }
    } else {
		qDebug() << "                              error message";
		qDebug() << ""; // newLine
		writeErrMessage(soc);
	}
}

/**
 * This method reads account and password from socket
 * @param soc
 * @return result of reading from socket
 */
bool Server::logIn(QTcpSocket *soc) {
	/* read user and password on socket*/
	qDebug() << "Server.cpp - logIn()     ---------- LOGIN ----------";

	/* usernameSize */
	readSpace(soc);
	int usernameSize = readNumberFromSocket(soc);
	qDebug() << "                         usernameSize: " << usernameSize;

	readSpace(soc);

	/* username */
	QString username;
	if (!readQString(soc, username, usernameSize)) {
		return false;
	}
	readSpace(soc);

	/* passwordSize */
	int passwordSize = readNumberFromSocket(soc);
	readSpace(soc);
	qDebug() << "                         passwordSize: " << passwordSize;

	QString password;
	if (!readQString(soc, password, passwordSize)) {
		return false;
	}

	qDebug() << "                         username: " << username << " password: " << password;
	qDebug() << ""; // newLine

	// Check if user is already logged in
	for (const std::pair<quintptr, QString> &pair : allUsernames) {
		if (pair.second == QString(username))
			return false;
	}

	// DB user authentication
	bool authentication = DB.authenticateUser(QString(username), QString(password));
	if (authentication) {
		usernames[soc->socketDescriptor()] = username;
        allUsernames[soc->socketDescriptor()] = username;
		return true;
	} else
		return false;
}

/**
 * This method reads username and password from the client for registration
 * @param soc
 * @return result of reading from socket
 */
bool Server::registration(QTcpSocket *soc) {
	qDebug() << "Server.cpp - registration()     ---------- REGISTRATION ----------";
	if (soc == nullptr) {
		return false;
	}
	readSpace(soc);
	int usernameSize = readNumberFromSocket(soc);
	readSpace(soc);

	//username
	QString username;
	if (!readQString(soc, username, usernameSize)) {
		return false;
	}
	readSpace(soc);

	int passwordSize = readNumberFromSocket(soc);
	readSpace(soc);

	//password
	QString password;
	if (!readQString(soc, password, passwordSize)) {
		return false;
	}
	readSpace(soc);

	/*QDataStream in(soc);
	qsizetype sizeAvatar;
	in >> sizeAvatar;*/
	int sizeAvatar = readNumberFromSocket(soc);
	readSpace(soc);

	qDebug() << "                                username: " << username << " size: " << usernameSize;
	qDebug() << "                                password: " << password << " size: " << passwordSize;
	qDebug() << "                                avatar size: " << sizeAvatar;

	//avatar
	QByteArray avatarDef;

	if (!readChunck(soc, avatarDef, sizeAvatar)) {
		return false;
	}

	qDebug() << "                                avatar size: " << sizeAvatar << " size read" << avatarDef.size();

	qDebug() << ""; // newLine

	bool registeredSuccessfully = DB.registerUser(QString(username), QString(password));
	if (registeredSuccessfully) {
		bool avatarChanged = DB.changeAvatar(QString(username), avatarDef);
        usernames[soc->socketDescriptor()] = username;
        allUsernames[soc->socketDescriptor()] = username;
		return true;

	} else
		return false;
}

/**
 * This method sends the user's object
 * @param soc
 * @return result of writing on socket
 */
bool Server::sendUser(QTcpSocket *soc) {
	qDebug() << "Server.cpp - sendUser()     ---------- SEND USER ----------";
	QByteArray message(AVATAR_MESSAGE);
	QByteArray image;
	//image = "image";
	QString username;
	username = usernames[soc->socketDescriptor()];
	image = DB.getAvatar(username);
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

/**
 * This method sends list of file names
 * @param soc
 * @return result of writing on socket
 */
bool Server::sendFileNames(QTcpSocket *soc) {
	qDebug() << "Server.cpp - sendFileNames()     ---------- LIST OF FILE ----------";


	//********************************* Versione finale ******************************************************
	// get username from map of logged in users
	QString username = allUsernames[soc->socketDescriptor()];
    std::map<QString, bool> files = DB.getFiles(username);
	int nFiles = files.size();
	QByteArray message(LIST_OF_FILE);
	QByteArray numFiles = convertionNumber(nFiles);

	message.append(" " + numFiles);

	for (std::pair<QString, bool> file : files) {
		QByteArray owner;
		owner.setNum(file.second ? 1 : 0);
		QByteArray filenameByteArray = convertionQString(file.first);
		QByteArray fileNameSize = convertionNumber(filenameByteArray.size());
		message.append(" " + fileNameSize + " " + filenameByteArray + " " + owner);
	}

    return  writeMessage(soc, message);
}

/**
 * This method reads user's file request
 * @param socketDescriptor
 * @param soc
 * @return result of reading from socket
 */
bool Server::readFileName(qintptr socketDescriptor, QTcpSocket *soc) {
	//std::lock_guard<std::mutex> lg(mutexThread);
	qDebug() << "Server.cpp - readFileName()     ---------- REQUEST FOR FILE ----------";
	readSpace(soc);
	int fileNameSize = readNumberFromSocket(soc);
	readSpace(soc);

	QString jsonFileName;
	if (!readQString(soc, jsonFileName, fileNameSize)) {
		return false;
	}

	qDebug() << "                               " << jsonFileName;

    QStringList fields = jsonFileName.split("%_##$$$##_%");

    if (fields.size() < 2) {
        QString owner = usernames[socketDescriptor];
        jsonFileName = owner + "%_##$$$##_%" + fields[0];
    } else{
        QString owner = fields[0];
        //fileName = fields[1];
    }

	QString username = usernames[socketDescriptor];
    QString key = jsonFileName;                           /* file name */

	auto result = threads.find(key);

	if (result != threads.end()) {
		/* file already open */
		qDebug() << "                               thread for file name aready exist " << jsonFileName;
		qDebug() << ""; // newLine
        std::unique_lock<std::shared_mutex> socketsLock(result->second->mutexSockets);
        std::unique_lock<std::shared_mutex> pendingSocketsLock(result->second->mutexPendingSockets);
        std::unique_lock<std::shared_mutex> usernamesLock(result->second->mutexUsernames);
        std::shared_lock<std::shared_mutex> filenameLock(result->second->mutexFilename);
        //soc->moveToThread(result->second.get());
		threads[key]->addSocket(soc, username);                       /* socket transition to secondary thread */
	} else {
		/* file not yet open */
		qDebug() << "                               New thread for file name: " << jsonFileName;
		qDebug() << ""; // newLine

		// First try to open requested file, else create a new one
		CRDT* loadedCrdt = new CRDT();
		Thread *thread;

		if (!loadedCrdt->loadCRDT(jsonFileName)) {
			qDebug() << "File need to be created";
			CRDT *crdt = new CRDT();
			DB.createFile(jsonFileName, usernames[socketDescriptor]);
			thread = new Thread(nullptr, crdt, jsonFileName, username, this);                        /* create new thread */
		} else
			thread = new Thread(nullptr, loadedCrdt, jsonFileName, username,
								this);                        /* create new thread */
		threads[key] = std::shared_ptr<Thread>(thread);

		// nel caso mettere i locks
		thread->addSocket(soc,
						  usernames[socketDescriptor]);                          /* socket transition to secondary thread */
		/*std::shared_ptr<Thread> thread = this->addThread(key, soc);
        thread->addSocket(soc, usernames[socketDescriptor]);*/
		thread->moveToThread(thread);
		//soc->moveToThread(thread);
		thread->start();
	}

	usernames.erase(socketDescriptor);
	//sockets.erase(socketDescriptor);
	return true;
}

/**
 * This methods reads user's changes from client
 * @param soc
 * @return
 */
bool Server::readEditAccount(QTcpSocket *soc) {
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

	if (DB.authenticateUser(usernames[soc->socketDescriptor()], oldPassword)) {
		if (newUsernameSize != 0) {
			if (DB.changeUsername(usernames[soc->socketDescriptor()], newUsername)) {
				QDir dir;
                dir.setNameFilters(QStringList(usernames[soc->socketDescriptor()] + "*"));
                dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

                qDebug() << "Scanning: " << dir.path();

                QStringList fileList = dir.entryList();
                for (int i=0; i<fileList.count(); i++){
                    QString filename = fileList[i].split("%_##$$$##_%")[1].split(".json")[0];
                    QString newFilename = newUsername + "%_##$$$##_%" + filename;
                    //auto thread = getThread(fileList[i].split(".json")[0]);
                    //if (thread != nullptr) {
                        // gestire concorrenza sul filename nella struttura del thread
                        //thread->changeFileName(newFilename);
                        changeNamethread(fileList[i].split(".json")[0], newFilename);

                    //}
                    qDebug() << "Found file: " << fileList[i];
                    QFile renamefile(fileList[i]);
                    
                    renamefile.rename(newFilename + ".json");
                    DB.changeFileName(fileList[i].split(".json")[0], newFilename);
                    renamefile.close();

                    /* trovare tutti gli utenti che hanno in comune i file dell'utente */
                    for (std::pair<qintptr, QString> username : allUsernames) {
                        if (username.first != soc->socketDescriptor()) {
                            std::map<QString, bool> listOfFile = DB.getFiles(username.second);

                            auto result = listOfFile.find(newFilename);
                            if (result != listOfFile.end()) {
                                if (sockets.find(username.first) != sockets.end())
                                    this->sendFileNames(sockets[username.first]);
                            }
                        }
                    }
                }

                usernames[soc->socketDescriptor()] = newUsername;
                addUsername(soc->socketDescriptor(), newUsername);
			} else {
				qDebug() << "Err1";
				return false;
			}
		}

		if (newPasswordSize != 0) {
			if (!DB.changePassword(usernames[soc->socketDescriptor()], newPassword)) {
				qDebug() << "Err2";
				return false;
			}
		}

		if (sizeAvatar != 0) {
			if (!DB.changeAvatar(usernames[soc->socketDescriptor()], avatarDef)) {
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

void Server::disconnected(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *connectDisconnected,
						  QTcpSocket *soc, qintptr socketDescriptor) {
    std::unique_lock<std::shared_mutex> allUsernamesMutex(mutexAllUsernames);
    qDebug() << "                              " << socketDescriptor << " Disconnected (form main thread)";
	qDebug() << ""; // newLine

	disconnect(*connectReadyRead);
	disconnect(*connectDisconnected);
	delete connectReadyRead;
	delete connectDisconnected;
	soc->deleteLater();
	socketsState.erase(socketDescriptor);
	usernames.erase(socketDescriptor);
    removeUsername(socketDescriptor);
}

/**
 *
 * @param fileName
 * @return Thread with a specific ID
 */
std::shared_ptr<Thread> Server::getThread(QString fileName) {
	auto result = threads.find(fileName);

	if (result != threads.end()) {
		return result.operator->()->second;
	} else {
		return std::shared_ptr<Thread>();
	}
}

/**
 * This method adds new Thread for specific file name
 * @param fileName
 * @return new Thread
 */
std::shared_ptr<Thread> Server::addThread(QString fileName, QString username) {
	CRDT* loadedCrdt = new CRDT();

	std::shared_ptr<Thread> thread;                        /* create new thread */
	if (!loadedCrdt->loadCRDT(fileName)) {
		qDebug() << "File need to be created";
		CRDT *crdt = new CRDT();
		DB.createFile(fileName, username);
		/* create new thread */
		thread = std::make_shared<Thread>(nullptr, crdt, fileName, username, this);
	} else {
		/* create new thread */
		thread = std::make_shared<Thread>(nullptr, loadedCrdt, fileName, username, this);
	}
	threads[fileName] = thread;
	return thread;
}

/**
 *  Read a shareCode from socket interact with DB
 * @param soc
 * @return
 */
bool Server::readShareCode(QTcpSocket *soc) {
	qDebug() << "Server.cpp - readShareCode()     ---------- READ SHARECODE ----------";
	readSpace(soc);
	int shareCodeSize = readNumberFromSocket(soc);
	readSpace(soc);

	QByteArray shareCode;
	if (!readChunck(soc, shareCode, shareCodeSize)) {
		return false;
	}

	qDebug() << shareCodeSize << shareCode;
	QString filename;

	if (handleShareCode(usernames[soc->socketDescriptor()], shareCode, filename)) {
		sendAddFile(soc, filename);
		return true;
	} else {
		writeErrMessage(soc, SHARE_CODE);
		return false;
	}
}

bool Server::handleShareCode(QString username, QString shareCode, QString &filename) {
	std::pair<QString, QString> pair = getInfoFromShareCode(shareCode);
	QString usernameOwner = pair.first;                    // TODO check problem in DB structure
	filename = pair.second;

	if (filename == "ERROR"){
	    return false;
	}

	if (DB.addPermission(filename, usernameOwner, username)) {
		return true;
	} else {
		return false;
	}
}

bool Server::sendAddFile(QTcpSocket *soc, QString filename) {
	QByteArray message(ADD_FILE);
	QByteArray filenameByteArray = convertionQString(filename);
	QByteArray fileNameSize = convertionNumber(filenameByteArray.size());
	QByteArray owner;
	owner.setNum(0);
	message.append(" " + fileNameSize + " " + filenameByteArray + " " + owner);

	if (!writeMessage(soc, message)) {
		return false;
	}

	return true;
}

/**
 * Retrieve username and filename from a given shareCode <--- username + "%_##$$$##_%" + filename
 * separator used: "%_##$$$##_%"
 * crypto key: 0xf55f15758b7e0153
 * @param shareCode : shareCode to decrypt
 * @return : pair <Username , Filename>
 */
std::pair<QString, QString> Server::getInfoFromShareCode(QString shareCode) {
	SimpleCrypt crypto;
	crypto.setKey(0xf55f15758b7e0153);

	QString decrypted = crypto.decryptToString(shareCode);

	QStringList fields = decrypted.split("%_##$$$##_%");

	qDebug() << fields;

	if (fields.size() != 3)
		return std::pair<QString, QString>("ERROR", "ERROR");
	else
		return std::pair<QString, QString>(fields[0], fields[1] + "%_##$$$##_%" + fields[2]);
}

Database Server::getDb() const {
	return DB;
}

bool Server::readRequestUsernameList(QTcpSocket *soc) {
    qDebug() << "Server.cpp - readRequestUsernameList()     ---------- REQUEST USERNAME LIST ----------";
    readSpace(soc);
    int fileNameSize = readNumberFromSocket(soc);
    readSpace(soc);

    QString jsonFileName;
    if (!readQString(soc, jsonFileName, fileNameSize)) {
        return false;
    }

    qDebug() << "                               " << jsonFileName;

    QStringList userlist = DB.getUsers(jsonFileName);
    qDebug() << userlist;


    int nFiles = userlist.size();
    QByteArray message(USERNAME_LIST_FOR_FILE);
    QByteArray numUsers = convertionNumber(nFiles);

    message.append(" " + numUsers);

    for (QString username : userlist){
        QByteArray usernameQBytearray = convertionQString(username);
        QByteArray usernameSize = convertionNumber(usernameQBytearray.size());
        message.append(" " + usernameSize + " " + usernameQBytearray);
    }

    return writeMessage(soc, message);
}

bool Server::readFileInformationChanges(QTcpSocket *soc){
    qDebug() << "Server.cpp - readFileInformationChanges()     ---------- READ FILE INFORMATION CHANGES ----------";
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

    qDebug() << usernamesSize;
    QStringList removedUsers;
    if (usernamesSize != 0){
        for (int i = 0; i < usernamesSize; i++){
            readSpace(soc);
            int usernameSize = readNumberFromSocket(soc);
            qDebug() << usernameSize;
            readSpace(soc);
            QString username;
            if (!readQString(soc, username, usernameSize)){
                return false;
            }
            qDebug() << "                              usename: "<<username;
            DB.removePermission(oldJsonFileName, username);
            removedUsers.append(username);
        }
    }

    qDebug() << oldJsonFileName << newJsonFileName;

    if (newFileNameSize != 0 && newJsonFileName != oldJsonFileName){
        QFile saveFile(oldJsonFileName + ".json");
        DB.changeFileName(oldJsonFileName, newJsonFileName);
        saveFile.rename(newJsonFileName+ ".json");
        saveFile.close();
        changeNamethread(oldJsonFileName, newJsonFileName);

        for (std::pair<qintptr, QString> username : allUsernames) {
            if (username.first != soc->socketDescriptor()) {
                std::map<QString, bool> listOfFile = DB.getFiles(username.second);

                auto result = listOfFile.find(newJsonFileName);
                if (result != listOfFile.end()) {
                    this->sendFileNames(sockets[username.first]);
                }
            }
        }

    }

    for (QString removedUsername : removedUsers) {
        for (std::pair<qintptr, QString> username : allUsernames) {
            if (username.first != soc->socketDescriptor() || removedUsername == username.second) {
                this->sendFileNames(sockets[username.first]);
            }
        }
    }

    auto thread = getThread(oldJsonFileName);
    if (thread != nullptr) {
        std::unique_lock<std::shared_mutex> threadSocketsMutex(thread->mutexSockets);
        std::shared_lock<std::shared_mutex> usernamesMutex(thread->mutexUsernames);
        std::unique_lock<std::shared_mutex> pendingSocketsMutex(thread->mutexPendingSockets);
        std::map<qintptr, QString> usernames = thread->getUsernames();
        std::map<qintptr, QTcpSocket *> threadSockets = thread->getSockets();

        for (QString removedUsername : removedUsers) {
            for (std::pair<qintptr, QString> username : usernames) {
                if (removedUsername == username.second){
                    thread->sendRemoveUser(username.first, username.second);
                    thread->addPendingSocket(username.first);
                    break;
                }
            }
        }
    }

    sendFileNames(soc);
    return true;
}

bool Server::readDeleteFile(QTcpSocket *soc) {
    qDebug() << "Server.cpp - readDeleteFile()     ---------- READ DELETE FILE ----------";
    readSpace(soc);
    int fileNameSize = readNumberFromSocket(soc);
    readSpace(soc);

    QString jsonFileName;
    if (!readQString(soc, jsonFileName, fileNameSize)) {
        return false;
    }

    qDebug() << "                               " << jsonFileName;

    std::map<QString, qintptr> usersremove;
    for (std::pair<qintptr, QString> username : allUsernames) {
        if (username.first != soc->socketDescriptor()) {
            std::map<QString, bool> listOfFile = DB.getFiles(username.second);

            auto result = listOfFile.find(jsonFileName);
            if (result != listOfFile.end()) {
                qDebug() << username.second;
                usersremove[username.second] = username.first;
            }
        }
    }

    if (!DB.deleteFile(jsonFileName)){
        return false;
    }

    QFile deleteFile(jsonFileName + ".json");
    deleteFile.remove();
    deleteFile.close();

    sendFileNames(soc);

    auto thread = getThread(jsonFileName);
    if (thread != nullptr) {
        std::unique_lock<std::shared_mutex> threadSocketsMutex(thread->mutexSockets);
        std::shared_lock<std::shared_mutex> usernamesMutex(thread->mutexUsernames);
        std::unique_lock<std::shared_mutex> pendingSocketsMutex(thread->mutexPendingSockets);
        std::unique_lock<std::shared_mutex> needToSaveMutex(thread->mutexNeedToSave);
        std::unique_lock<std::shared_mutex> fileDeletedMutex(thread->mutexFileDeleted);

        auto sockets = thread->getSockets();

        for (std::pair<qintptr, QTcpSocket *> socket : sockets) {
            if (soc->socketDescriptor() != socket.first) {
                this->sendFileNames(socket.second);
            }
        }
        thread->deleteFile();
        addDeleteFileThread(jsonFileName);
    }

    for (std::pair<QString, qintptr> user : usersremove) {
        if (!sendFileNames(this->sockets[user.second])) {
                return false;
        }
    }

    return true;
}

const std::map<qintptr, QTcpSocket *> &Server::getSockets() const {
    return sockets;
}

void Server::removeUsername(qintptr socketdescriptor) {
    allUsernames.erase(socketdescriptor);
}

void Server::removeSocket(qintptr socketDescriptor){
    sockets.erase(socketDescriptor);
}

void Server::addUsername(qintptr socketdescriptor, QString username) {
    allUsernames[socketdescriptor] = username;
}

const std::map<qintptr, QString> &Server::getUsernames() const {
    return usernames;
}

const std::map<qintptr, QString> &Server::getAllUsernames() const {
    return allUsernames;
}

void Server::changeNamethread(QString oldFilename, QString newFilename){
    auto result = threads.find(oldFilename);

    if (result != threads.end()) {
        threads[newFilename] = result->second;
        threads.erase(oldFilename);
        std::unique_lock<std::shared_mutex> filenameLock(result->second->mutexFilename);
        result->second->changeFileName(newFilename);
    }
}

void Server::addDeleteFileThread(QString filename){
    if (threads.find(filename) != threads.end()) {
        deleteFileThread[filename] = threads[filename];
        threads.erase(filename);
    }
}

void Server::removeDeleteFileThread(QString filename){
    deleteFileThread.erase(filename);
}

const std::shared_mutex &Server::getMutexAllUsernames()  {
    return mutexAllUsernames;
}

const std::shared_mutex &Server::getMutexUsernames()  {
    return mutexUsernames;
}
