#ifndef THREAD_H
#define THREAD_H
#include <QThread>
#include <QTcpSocket>
#include "../Utils/Constants.h"
#include "../../client/utils/Identifier.h"
#include "common/commonFunctions.h"

class Thread : public QThread{
Q_OBJECT
private:
    std::map<qintptr, std::shared_ptr<QTcpSocket>> sockets;

public:
    explicit Thread(QObject *parent = nullptr);
    void run();
    void addSocket(qintptr socketDescriptor);
private:
    bool readInsert(QTcpSocket *soc);
    bool readDelete(QTcpSocket *soc);
    void insert(QString str, QString siteId, std::vector<int> pos);
    void deleteChar(QString str, QString siteId, std::vector<int> pos);

signals:
    void error(QTcpSocket::SocketError socketerror);

public slots:
    void readyRead(QTcpSocket *socket);
    void disconnected(QTcpSocket *socket);
};

#endif // THREAD_H
