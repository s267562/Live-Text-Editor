#include <QDataStream>
#include <QPixmap>
#include <shared_mutex>
#include "Server.h"

Server::Server(QObject *parent) : QTcpServer(parent) {}

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

void Server::connection() {
	QTcpSocket *soc = this->nextPendingConnection();

	qintptr socketDescriptor = soc->socketDescriptor();
	socketsState[soc->socketDescriptor()] = UNLOGGED;
	QMetaObject::Connection *c = new QMetaObject::Connection();
	QMetaObject::Connection *d = new QMetaObject::Connection();

	*c = connect(soc, &QTcpSocket::readyRead, this, [this, c, d, soc, socketDescriptor] {
		QByteArray data;
		if (!readChunck(soc, data, 5)) {
			writeErrMessage(soc);
			soc->flush();
			return;
		}

		qDebug() << "Server.cpp - connection()     msg received:" << data;

		if (data.toStdString() == LOGIN_MESSAGE) {
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
			if (registration(soc)) {
			    sendUser(soc);
				sendFileNames(soc);
				socketsState[socketDescriptor] = LOGGED;
			} else {
				writeErrMessage(soc, REGISTRATION_MESSAGE);
				return;
			}
		} else if (data.toStdString() == REQUEST_FILE_MESSAGE && socketsState[socketDescriptor] == LOGGED) {
			/* disconnect from main thread */
			disconnect(*c);
			disconnect(*d);
			delete c;
			delete d;
			socketsState.erase(socketDescriptor);
			qDebug() << "                              socketsSize: " << socketsState.size();
			qDebug() << ""; // newLine

			if (!readFileName(soc->socketDescriptor(), soc)) {
				writeErrMessage(soc, REQUEST_FILE_MESSAGE);
			}
		}else if (data.toStdString() == EDIT_ACCOUNT && socketsState[socketDescriptor] == LOGGED){
			if (readEditAccount(soc)){
				//writeOkMessage(soc);
				sendUser(soc);
			}else{
				writeErrMessage(soc, EDIT_ACCOUNT);
			}
		}else {
			qDebug() << "                              error message";
			qDebug() << ""; // newLine
			writeErrMessage(soc);
		}
	}, Qt::DirectConnection);

	*d = connect(soc, &QTcpSocket::disconnected, this, [this, c, d, soc, socketDescriptor] {
		qDebug() << "                              " << socketDescriptor << " Disconnected (form main thread)";
		qDebug() << ""; // newLine
		soc->deleteLater();
		socketsState.erase(socketDescriptor);
		usernames.erase(socketDescriptor);
	});
}

bool Server::logIn(QTcpSocket *soc) {
	/* read user and password on socket*/
	qDebug() << "Server.cpp - logIn()     ---------- LOGIN ----------";

	/* usernameSize */
	readSpace(soc);
	int usernameSize = readNumberFromSocket(soc);
	qDebug() << "                         usernameSize: " << usernameSize;

	readSpace(soc);
	/* username */
	QByteArray username;
	if (!readChunck(soc, username, usernameSize)) {
		return false;
	}
	readSpace(soc);

	/* passwordSize */
	int passwordSize = readNumberFromSocket(soc);
	readSpace(soc);
	qDebug() << "                         passwordSize: " << passwordSize;

	QByteArray password;
	if (!readChunck(soc, password, passwordSize)) {
		return false;
	}

	qDebug() << "                         username: " << username << " password: " << password;
	qDebug() << ""; // newLine

	// Check if user is already logged in
	for (std::pair<quintptr, QString> pair : usernames) {
		if (pair.second == QString(username))
			return false;
	}

	// DB user authentication
	bool authentication = DB.authenticateUser(QString(username), QString(password));
	if (authentication) {
		usernames[soc->socketDescriptor()] = username;
		return true;
	} else
		return false;
}

bool Server::sendUser(QTcpSocket *soc){
	qDebug() << "Server.cpp - sendUser()     ---------- SEND USER ----------";
	QByteArray message(AVATAR_MESSAGE);
	QByteArray image;
	//image = "image";
	QString username;
	username = usernames[soc->socketDescriptor()];
	image = DB.getAvatar(username);
	QByteArray imageSize = convertionNumber(image.size());
	QByteArray usernameSize = convertionNumber(username.size());

	message.append(" " + usernameSize + " " + username.toUtf8() + " " + imageSize + " " + image);

	qDebug() << message;

	if (!writeMessage(soc, message)){
		return false;
	}

	return true;
}

bool Server::sendFileNames(QTcpSocket *soc) {
	qDebug() << "Server.cpp - sendFileNames()     ---------- LIST OF FILE ----------";
	// TODO gestione file dell'utente

	//********************************* Versione finale ******************************************************
	// get username from map of logged in users
//	QString username = usernames[soc->socketDescriptor()];
//	QList<QString> files = DB.getFiles("");
//	int nFiles = files.size();
//	QByteArray message(LIST_OF_FILE);
//	QByteArray numFiles = convertionNumber(nFiles);
//
//	message.append(" " + numFiles);
//
//	for (QString filename : files) {
//		QByteArray fileNameSize = convertionNumber(filename.size());
//		message.append(" " + fileNameSize + " " + filename.toUtf8());
//	}
	//******************************************************************************************************

	//******************************************* Versione dtest senza DB***********************************
	int nFiles = 2;
	std::list<std::pair<QString, bool>> files;								/* files fantoccio */
	files.push_back(std::make_pair("file1", true));
	files.push_back(std::make_pair("file2", false));

	QByteArray message(LIST_OF_FILE);

	QByteArray numFiles = convertionNumber(nFiles);
	message.append(" " + numFiles);

	for (std::pair<QString, bool> file : files) {
		QByteArray fileNameSize = convertionNumber(file.first.size());
		QByteArray shared;
		shared.setNum(file.second ? 1 : 0);
		message.append(" " + fileNameSize + " " + file.first.toUtf8() + " " + shared);
	}

	qDebug() << "                                " << message;
	qDebug() << ""; // newLine
	//******************************************************************************************************

	writeMessage(soc, message);
	if (nFiles == 0)
		return false;
	else
		return true;
}

bool Server::readFileName(qintptr socketDescriptor, QTcpSocket *soc) {
	std::lock_guard<std::mutex> lg(mutexThread);
	qDebug() << "Server.cpp - readFileName()     ---------- REQUEST FOR FILE ----------";
	readSpace(soc);
	int fileNameSize = readNumberFromSocket(soc);
	readSpace(soc);

	QByteArray fileName;
	if (!readChunck(soc, fileName, fileNameSize)) {
		return false;
	}

	qDebug() << "                               " << fileName;

	QString key = fileName;                           /* file name */
	auto result = threads.find(key);

	if (result != threads.end()) {
		/* file already open */
		qDebug() << "                               thread for file name aready exist " << fileName;
		qDebug() << ""; // newLine
		threads[key]->addSocket(soc,
								usernames[socketDescriptor]);                       /* socket transition to secondary thread */
	} else {
		/* file not yet open */
		qDebug() << "                               New thread for file name: " << fileName;
		qDebug() << ""; // newLine
		CRDT *crdt = new CRDT();
		Thread *thread = new Thread(this, crdt, fileName, this);                        /* create new thread */
		threads[key] = std::shared_ptr<Thread>(thread);
		thread->addSocket(soc,
						  usernames[socketDescriptor]);                            /* socket transition to secondary thread */
		thread->start();
	}

	usernames.erase(socketDescriptor);

	return true;
}

bool Server::registration(QTcpSocket *soc) {
	qDebug() << "Server.cpp - registration()     ---------- REGISTRATION ----------";
	if (soc == nullptr) {
		return false;
	}
	readSpace(soc);
	int sizeUsername = readNumberFromSocket(soc);
	readSpace(soc);

	//username
	QByteArray username;
	if (!readChunck(soc, username, sizeUsername)) {
		return false;
	}
	readSpace(soc);

	int sizePassword = readNumberFromSocket(soc);
	readSpace(soc);

	//password
	QByteArray password;
	if (!readChunck(soc, password, sizePassword)) {
		return false;
	}
	readSpace(soc);

	/*QDataStream in(soc);
	qsizetype sizeAvatar;
	in >> sizeAvatar;*/
	int sizeAvatar = readNumberFromSocket(soc);
	readSpace(soc);

	qDebug() << "                                username: " << username << " size: " << sizeUsername;
	qDebug() << "                                password: " << password << " size: " << sizePassword;
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

		//qDebug() << "Registered succesfully";
		//if(avatarChanged)
		//	qDebug() << "Avatar changed";


		usernames[soc->socketDescriptor()] = username;
		return true;

	} else
		return false;
}

bool Server::readEditAccount(QTcpSocket *soc) {
	qDebug() << "Server.cpp - readEditAccount()     ---------- READ EDIT ACCOUNT ----------";
	if (soc == nullptr) {
		return false;
	}
	readSpace(soc);
	int sizeUsername = readNumberFromSocket(soc);
	readSpace(soc);

	//username
	QByteArray username;
	if (!readChunck(soc, username, sizeUsername)) {
		return false;
	}
	readSpace(soc);

	int newPasswordSize = readNumberFromSocket(soc);
	readSpace(soc);

	//password
	QByteArray newPassword;
	if (!readChunck(soc, newPassword, newPasswordSize)) {
		return false;
	}
	readSpace(soc);

	int oldPasswordSize = readNumberFromSocket(soc);
	readSpace(soc);

	//password
	QByteArray oldPassword;
	if (!readChunck(soc, oldPassword, oldPasswordSize)) {
		return false;
	}
	readSpace(soc);


	int sizeAvatar = readNumberFromSocket(soc);
	readSpace(soc);

	qDebug() << "                                username: " << username << " size: " << sizeUsername;
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
		if (sizeUsername != 0) {
			if (DB.changeUsername(usernames[soc->socketDescriptor()], username)) {
				usernames[soc->socketDescriptor()] = username;
			}else{
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
	}else{
		qDebug() << "Err4";
		return false;
	}

	return true;
}

std::shared_ptr<Thread> Server::getThread(QString fileName) {
	std::lock_guard<std::mutex> sl(mutexThread);
	auto result = threads.find(fileName);

	if (result != threads.end()) {
		return result.operator->()->second;
	} else {
		return std::shared_ptr<Thread>();
	}
}

std::shared_ptr<Thread> Server::addThread(QString fileName) {
	std::lock_guard<std::mutex> lg(mutexThread);
	CRDT *crdt = new CRDT();
	std::shared_ptr<Thread> thread = std::make_shared<Thread>(this, crdt, fileName,
															  this);                        /* create new thread */
	threads[fileName] = thread;
	return thread;
}