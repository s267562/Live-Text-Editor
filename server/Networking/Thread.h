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
    std::map<qintptr, QTcpSocket*> sockets;  /* TO-DO: sincronizzazione con il thread principale */
    std::map<qintptr, QString> usernames;
    std::mutex mutexSockets;
    std::queue<Message> messagesQueue;
    CRDT *crdt;
    QString filename;
    QTimer *saveTimer;
    Server *server;
    int saveInterval = 2 * /*60 **/ 1000; // 2 min (in ms) // TODO decidere intervallo
    bool needToSaveFile = false;
    bool timerStarted = false;

public:
    explicit Thread(QObject *parent = nullptr, CRDT *crdt = nullptr, QString filename = "", Server *server = nullptr);
    void run();
    void addSocket(QTcpSocket *soc, QString username);
    void sendListOfUsers(QTcpSocket *soc);

private:
    bool readInsert(QTcpSocket *soc);
    bool readStyleChanged(QTcpSocket *soc);
    bool readDelete(QTcpSocket *soc);
    void writeInsert(QTcpSocket *soc, Character character);
    void writeStyleChanged(QTcpSocket *soc, Character character);
    void writeDelete(QTcpSocket *soc, Character character);
    void sendNewUser(QTcpSocket *soc);
    void sendRemoveUser(qintptr socketDescriptor, QString username);
    void sendFile(QTcpSocket *soc);
    bool readShareCode(QTcpSocket *soc);
    bool sendAddFile(QTcpSocket *soc, QString filename);

signals:
    void error(QTcpSocket::SocketError socketerror);
    void newMessage();

public slots:
    void readyRead(QTcpSocket *soc, QMetaObject::Connection *c, QMetaObject::Connection *d);
    void disconnected(QTcpSocket *socket, qintptr socketDescriptor, QMetaObject::Connection *c, QMetaObject::Connection *d);
    void saveCRDTToFile();
};

#endif // THREAD_H
