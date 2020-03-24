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
#include "../CRDT/CRDT.h"
#include "Server.h"
#include <shared_mutex>

class Identifier;

class Character;

class Server;

class Thread : public QThread {
Q_OBJECT
private:
    /* ATTRIBUTES */
    std::map<qintptr, QTcpSocket *> sockets;                        // sync
	std::map<qintptr, QString> usernames;                           // sync
	std::map<qintptr, QTcpSocket *> pendingSocket;                  // sync
	CRDT *crdt;
	QString filename;                                               // sync
	QString usernameOwner;
	QTimer *saveTimer;
	Server *server;
	int saveInterval = 2 * /*60 **/ 1000; // (in ms) // TODO decidere intervallo
	bool needToSaveFile = false;
	bool timerStarted = false;
	bool fileDeleted = false;

    /* METHODS */
    bool readInsert(QTcpSocket *soc);
    bool readStyleChanged(QTcpSocket *soc);
    bool readDelete(QTcpSocket *soc);
    bool readAlignmentChanged(QTcpSocket *soc);
    bool writeInsert(QTcpSocket *soc, Character& character);
    bool writeStyleChanged(QTcpSocket *soc, Character& character);
    bool writeDelete(QTcpSocket *soc, Character& character);
    bool writeAlignmentChanged(QTcpSocket *soc, int alignment, Character& blockId);
    bool sendNewUser(QTcpSocket *soc);                                              // sync ok
    bool sendFile(QTcpSocket *soc);
    bool readShareCode(QTcpSocket *soc);                                            // sync ok
    bool sendAddFile(QTcpSocket *soc, QString filename);
    bool readEditAccount(QTcpSocket *soc);                                          // sync ok
    bool sendUser(QTcpSocket *soc);                                                 // sync ok
    bool readRequestUsernameList(QTcpSocket *soc);
    bool readFileInformationChanges(QTcpSocket *soc);                               // sync ok
    bool readDeleteFile(QTcpSocket *soc);                                           // sync ok
    bool readFileName(QTcpSocket *soc, QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *connectDisconnected);

    void connectSlot(QTcpSocket *soc, QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *connectDisconnected);

    static bool renameFileSave(QString oldFilename,QString newFilename);
    static bool deleteFileSave(QString filename);

public:
    /* ATTRIBUTES */
    std::shared_mutex mutexSockets;
    std::shared_mutex mutexUsernames;
    std::shared_mutex mutexPendingSockets;
    std::shared_mutex mutexFilename;
    std::shared_mutex mutexNeedToSave;
    std::shared_mutex mutexFileDeleted;

    /* METHODS */
    explicit Thread(QObject *parent = nullptr, CRDT *crdt = nullptr, QString filename = "", QString usernameOwner = "",
					Server *server = nullptr);
	void run();
	bool addSocket(QTcpSocket *soc, QString username);                              // sync ok
	bool sendListOfUsers(QTcpSocket *soc);                                          // sync ok
    std::map<qintptr, QTcpSocket *> getSockets();                                   // sync
    void changeFileName(QString filename);                                          // sync ok
    bool sendRemoveUser(qintptr socketDescriptor, QString username);               // sync ok
    void addPendingSocket(qintptr socketDescriptor);                                // sync ok
    const std::map<qintptr, QString> &getUsernames() const;                         // sync ok
    void deleteFile();                                                              // sync ok

signals:
    void error(QTcpSocket::SocketError socketerror);
	void newMessage();
	void removeThread(QString filename);

public slots:
	void readyRead(QTcpSocket *soc, QMetaObject::Connection *c, QMetaObject::Connection *d);                                            // sync
	void
	disconnected(QTcpSocket *socket, qintptr socketDescriptor, QMetaObject::Connection *c, QMetaObject::Connection *d);                 // sync
	void saveCRDTToFile();                                                                                                              // sync ok
};

#endif // THREAD_H