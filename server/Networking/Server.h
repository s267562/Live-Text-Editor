#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include "Thread.h"
#include "../Utils/Constants.h"
#include "common/commonFunctions.h"
#include "../DB/Database.h"

class Thread;

class Server: public QTcpServer{
    Q_OBJECT
private:
    std::map<QString,std::shared_ptr<Thread>> threads;
    std::mutex mutexThread;
    std::map<qintptr, SocketState> socketsState;
    std::map<qintptr, QString> usernames;
    QTcpSocket *socket;
    Database DB;

public:
    explicit Server(QObject *parent = nullptr);
    bool startServer(quint16 port);
    std::shared_ptr<Thread> getThread(QString fileName);
    std::shared_ptr<Thread> addThread(QString fileName);

private:
    bool logIn(QTcpSocket *soc);
    bool sendFileNames(QTcpSocket *soc);
    bool readFileName(qintptr socketDescriptor, QTcpSocket *soc);
    bool registration(QTcpSocket *soc);

signals:

public slots:
    void connection();
};

#endif // SERVER_H
