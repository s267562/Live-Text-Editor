#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include "Thread.h"
#include "../../common/Constants.h"
#include "../../common/commonFunctions.h"
#include "../DB/Database.h"

class Thread;

class Server: public QTcpServer{
    Q_OBJECT
private:
    std::map<QString,std::shared_ptr<Thread>> threads;                       // sync
    std::map<QString,std::shared_ptr<Thread>> deleteFileThread;              // sync
    std::mutex mutexThread;
    std::map<qintptr, SocketState> socketsState;
    std::map<qintptr, QString> usernames;                                    // sync ??
    std::map<qintptr, QString> allUsernames;                                 // sync
    std::map<qintptr, QTcpSocket*> sockets;                                  // sync
    Database DB;

public:
    explicit Server(QObject *parent = nullptr);
    bool startServer(quint16 port);
    std::shared_ptr<Thread> getThread(QString fileName);
    std::shared_ptr<Thread> addThread(QString fileName, QString username);
    bool handleShareCode(QString username, QString shareCode, QString &filename);
    Database getDb() const;
    bool sendFileNames(QTcpSocket *soc);
    const std::map<qintptr, QTcpSocket *> &getSockets() const;
    void removeUsername(qintptr username);
    void changeNamethread(QString oldFilename, QString newFilename);
    const std::map<qintptr, QString> &getUsernames() const;
    const std::map<qintptr, QString> &getAllUsernames() const;
    void addUsername(qintptr socketdescriptor, QString username);
    void removeSocket(qintptr socketDescriptor);
    void addDeleteFileThread(QString filename);
    void removeDeleteFileThread(QString filename);

private:
    void readyRead(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *disconnectReadyRead, QTcpSocket* soc, qintptr socketDescriptor);
    void disconnected(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *disconnectReadyRead, QTcpSocket* soc, qintptr socketDescriptor);
    bool logIn(QTcpSocket *soc);                                        // sync
    bool sendUser(QTcpSocket *soc);                                 // sync  ??
    bool readFileName(qintptr socketDescriptor, QTcpSocket *soc);
    bool registration(QTcpSocket *soc);                                 // sync
    bool readEditAccount(QTcpSocket *soc);
    static std::pair<QString,QString> getInfoFromShareCode(QString shareCode);
    bool readShareCode(QTcpSocket *soc);
    bool sendAddFile(QTcpSocket *soc, QString filename);
    bool readRequestUsernameList(QTcpSocket *soc);
    bool readFileInformationChanges(QTcpSocket *soc);
    bool readDeleteFile(QTcpSocket *soc);

signals:

public slots:
    void connection();
};

#endif // SERVER_H
