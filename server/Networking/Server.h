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
    Database DB;                                                             // rendere le operazioni atomiche

public:
    explicit Server(QObject *parent = nullptr);
    bool startServer(quint16 port);
    std::shared_ptr<Thread> getThread(QString fileName);                            // sync
    std::shared_ptr<Thread> addThread(QString fileName, QString username);          // sync
    bool handleShareCode(QString username, QString shareCode, QString &filename);
    Database getDb() const;
    bool sendFileNames(QTcpSocket *soc);                                            // sync
    const std::map<qintptr, QTcpSocket *> &getSockets() const;                      // sync
    void removeUsername(qintptr username);                                          // sync
    void changeNamethread(QString oldFilename, QString newFilename);                // sync
    const std::map<qintptr, QString> &getUsernames() const;                         // sync
    const std::map<qintptr, QString> &getAllUsernames() const;                      // sync
    void addUsername(qintptr socketdescriptor, QString username);                   // sync
    void removeSocket(qintptr socketDescriptor);                                    // sync
    void addDeleteFileThread(QString filename);                                     // sync
    void removeDeleteFileThread(QString filename);

private:
    void readyRead(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *disconnectReadyRead, QTcpSocket* soc, qintptr socketDescriptor);                     // sync
    void disconnected(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *disconnectReadyRead, QTcpSocket* soc, qintptr socketDescriptor);                  // sync
    bool logIn(QTcpSocket *soc);                                        // sync
    bool sendUser(QTcpSocket *soc);                                 // sync  ??
    bool readFileName(qintptr socketDescriptor, QTcpSocket *soc);
    bool registration(QTcpSocket *soc);                                 // sync
    bool readEditAccount(QTcpSocket *soc);                              // sync
    static std::pair<QString,QString> getInfoFromShareCode(QString shareCode);
    bool readShareCode(QTcpSocket *soc);                                // sync
    bool sendAddFile(QTcpSocket *soc, QString filename);
    bool readRequestUsernameList(QTcpSocket *soc);
    bool readFileInformationChanges(QTcpSocket *soc);                   // sync
    bool readDeleteFile(QTcpSocket *soc);                               // sync

signals:

public slots:
    void connection();
};

#endif // SERVER_H
