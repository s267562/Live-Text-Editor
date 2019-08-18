#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include "Thread.h"
#include "../Utils/Constants.h"
#include "common/commonFunctions.h"

class Server: public QTcpServer{
    Q_OBJECT
private:
    std::map<std::string,std::shared_ptr<Thread>> threads;
    std::map<qintptr, SocketState> socketsState;
    QTcpSocket *socket;

public:
    explicit Server(QObject *parent = nullptr);
    void startServer(quint16 port);

private:
    bool logIn(QTcpSocket *soc);
    bool readFileName(qintptr socketDescriptor, QTcpSocket *soc);
    bool registration(QTcpSocket *soc);

signals:

public slots:
    void connection();
};

#endif // SERVER_H
