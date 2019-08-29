#ifndef THREAD_H
#define THREAD_H
#include <QThread>
#include <QTcpSocket>
#include <queue>
#include "../Utils/Constants.h"
#include "../../client/utils/Identifier.h"
#include "../../client/utils/Character.h"
#include "common/commonFunctions.h"
#include "message/Message.h"
#include "../../client/utils/Pos.h"
#include "../CRDT.h"

class Identifier;
class Character;

class Thread : public QThread{
Q_OBJECT
private:
    std::map<qintptr, std::shared_ptr<QTcpSocket>> sockets;  /* TO-DO: sincronizzazione con il thread principale */
    std::queue<Message> messagesQueue;
    CRDT *crdt;

public:
    explicit Thread(QObject *parent = nullptr, CRDT *crdt = nullptr);
    void run();
    void addSocket(QTcpSocket *soc);
private:
    bool readInsert(QTcpSocket *soc);
    bool readDelete(QTcpSocket *soc);
    void insert(QString str, QString siteId, std::vector<Identifier> pos);
    void deleteChar(QString str, QString siteId, std::vector<int> pos);
    void deleteChar(QString str, QString siteId, std::vector<Identifier> pos);

signals:
    void error(QTcpSocket::SocketError socketerror);
    void newMessage();

public slots:
    void readyRead(QTcpSocket *socket);
    void disconnected(QTcpSocket *socket, qintptr socketDescriptor);
};

#endif // THREAD_H
