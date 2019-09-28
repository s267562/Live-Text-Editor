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
    bool handleShareCode(QString username, QString shareCode, QString &filename);

private:
    void readyRead(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *disconnectReadyRead, QTcpSocket* soc, qintptr socketDescriptor);
    void disconnected(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *disconnectReadyRead, QTcpSocket* soc, qintptr socketDescriptor);
    bool logIn(QTcpSocket *soc);
    bool sendUser(QTcpSocket *soc);
    bool sendFileNames(QTcpSocket *soc);
    bool readFileName(qintptr socketDescriptor, QTcpSocket *soc);
    bool registration(QTcpSocket *soc);
    bool readEditAccount(QTcpSocket *soc);
    std::pair<QString,QString> getInfoFromShareCode(QString shareCode);
    bool readShareCode(QTcpSocket *soc);
    bool sendAddFile(QTcpSocket *soc, QString filename);

signals:

public slots:
    void connection();
};

#endif // SERVER_H
