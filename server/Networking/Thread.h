#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QTcpSocket>
#include <queue>
#include <QtCore/QTimer>
#include "../../common/Constants.h"
#include "../../client/utils/Identifier.h"
#include "../../client/utils/Character.h"
#include "../../common/commonFunctions.h"
#include "../../common/Message.h"
#include "../../client/utils/Pos.h"
#include "../CRDT.h"
#include "Server.h"

class Identifier;

class Character;

class Server;

class Thread : public QThread {
Q_OBJECT
private:
	std::map<qintptr, QTcpSocket *> sockets;                 // sync
	std::map<qintptr, QString> usernames;               // sync
	std::map<qintptr, QTcpSocket *> pendingSocket;               // sync
    std::mutex mutexSockets;
	CRDT *crdt;
	QString filename;               // sync
	QString usernameOwner;
	QTimer *saveTimer;
	Server *server;
	int saveInterval = 2 * /*60 **/ 1000; // 2 min (in ms) // TODO decidere intervallo
	bool needToSaveFile = false;
	bool timerStarted = false;
	bool fileDeleted = false;

public:
	explicit Thread(QObject *parent = nullptr, CRDT *crdt = nullptr, QString filename = "", QString usernameOwner = "",
					Server *server = nullptr);

	void run();

	void addSocket(QTcpSocket *soc, QString username);               // sync

	void sendListOfUsers(QTcpSocket *soc);               // sync

    std::map<qintptr, QTcpSocket *> getSockets();               // sync

    void changeFileName(QString filename);               // sync

    void sendRemoveUser(qintptr socketDescriptor, QString username);               // sync

    void addPendingSocket(qintptr socketDescriptor);               // sync

    const std::map<qintptr, QString> &getUsernames() const;               // sync

    void deleteFile();               // sync

private:
	bool readInsert(QTcpSocket *soc);

	bool readStyleChanged(QTcpSocket *soc);

	bool readDelete(QTcpSocket *soc);

	void writeInsert(QTcpSocket *soc, Character character);

	void writeStyleChanged(QTcpSocket *soc, Character character);

	void writeDelete(QTcpSocket *soc, Character character);

	void sendNewUser(QTcpSocket *soc);               // sync

	void sendFile(QTcpSocket *soc);

	bool readShareCode(QTcpSocket *soc);               // sync

	bool sendAddFile(QTcpSocket *soc, QString filename);

	bool readEditAccount(QTcpSocket *soc);               // sync

	bool sendUser(QTcpSocket *soc);               // sync

    bool readRequestUsernameList(QTcpSocket *soc);

    bool readFileInformationChanges(QTcpSocket *soc);               // sync

    bool readDeleteFile(QTcpSocket *soc);               // sync

signals:

	void error(QTcpSocket::SocketError socketerror);

	void newMessage();

public slots:

	void readyRead(QTcpSocket *soc, QMetaObject::Connection *c, QMetaObject::Connection *d);                // sync

	void
	disconnected(QTcpSocket *socket, qintptr socketDescriptor, QMetaObject::Connection *c, QMetaObject::Connection *d);                // sync

	void saveCRDTToFile();                // sync
};

#endif // THREAD_H
