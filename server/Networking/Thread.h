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

class Identifier;
class Character;

class Thread : public QThread{
Q_OBJECT
private:
    std::map<qintptr, std::shared_ptr<QTcpSocket>> sockets;  /* TO-DO: sincronizzazione con il thread principale */
    std::queue<Message> messagesQueue;

public:
    explicit Thread(QObject *parent = nullptr);
    void run();
    void addSocket(qintptr socketDescriptor);
private:
    bool readInsert(QTcpSocket *soc);
    bool readDelete(QTcpSocket *soc);
    void insert(QString str, QString siteId, std::vector<int> pos);
    void insert(QString str, QString siteId, std::vector<Identifier> pos);
    void deleteChar(QString str, QString siteId, std::vector<int> pos);
    void deleteChar(QString str, QString siteId, std::vector<Identifier> pos);

signals:
    void error(QTcpSocket::SocketError socketerror);

public slots:
    void readyRead(QTcpSocket *socket);
    void disconnected(QTcpSocket *socket, qintptr socketDescriptor);
};

#endif // THREAD_H
