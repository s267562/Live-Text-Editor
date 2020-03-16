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
#include <shared_mutex>

class Thread;

class Server: public QTcpServer{
    Q_OBJECT
private:
    std::map<QString,std::shared_ptr<Thread>> threads;                       // sync
    std::map<QString,std::shared_ptr<Thread>> deleteFileThread;              // sync
    std::map<qintptr, SocketState> socketsState;
    std::map<qintptr, QString> usernames;                                    // sync ??
    std::map<qintptr, QString> allUsernames;                                 // sync
    std::map<qintptr, QTcpSocket*> sockets;                                  // sync
    Database DB;                                                             // rendere le operazioni atomiche

public:
    std::shared_mutex mutexThread;
    std::shared_mutex mutexDeleteFileThread;
    std::shared_mutex mutexSockets;
    std::shared_mutex mutexUsernames;
    std::shared_mutex mutexAllUsernames;

public:
    explicit Server(QObject *parent = nullptr);
    bool startServer(quint16 port);
    std::shared_ptr<Thread> getThread(QString fileName);                            // sync
    std::shared_ptr<Thread> addThread(QString fileName, QString username);          // sync
    bool handleShareCode(QString username, QString shareCode, QString &filename);
    Database getDb() const;
    bool sendFileNames(QTcpSocket *soc);                                            // sync ok
    const std::map<qintptr, QTcpSocket *> &getSockets() const;                      // sync
    void removeUsername(qintptr username);                                          // sync ok
    void changeNamethread(QString oldFilename, QString newFilename);                // sync -> da fare
    const std::map<qintptr, QString> &getUsernames() const;                         // sync
    const std::map<qintptr, QString> &getAllUsernames() const;                      // sync
    void addUsername(qintptr socketdescriptor, QString username);                   // sync ok
    void removeSocket(qintptr socketDescriptor);                                    // sync ok
    void addDeleteFileThread(QString filename);                                     // sync ok
    void removeDeleteFileThread(QString filename);                                  // sync -> da fare


private:
    void readyRead(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *disconnectReadyRead, QTcpSocket* soc, qintptr socketDescriptor);                     // sync
    void disconnected(QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *disconnectReadyRead, QTcpSocket* soc, qintptr socketDescriptor);                  // sync
    bool logIn(QTcpSocket *soc);                                        // sync ok
    bool sendUser(QTcpSocket *soc);                                     // sync  ok
    bool readFileName(qintptr socketDescriptor, QTcpSocket *soc);       // sync ok
    bool registration(QTcpSocket *soc);                                 // sync ok
    bool readEditAccount(QTcpSocket *soc);                              // sync ok, fare anche per il filename nella struttura del thread
    static std::pair<QString,QString> getInfoFromShareCode(QString shareCode);
    bool readShareCode(QTcpSocket *soc);                                // sync ok
    bool sendAddFile(QTcpSocket *soc, QString filename);
    bool readRequestUsernameList(QTcpSocket *soc);
    bool readFileInformationChanges(QTcpSocket *soc);                   // sync ok
    bool readDeleteFile(QTcpSocket *soc);                               // sync ok

    void connectionSlot(QTcpSocket *soc, QMetaObject::Connection *connectReadyRead, QMetaObject::Connection *connectDisconnected);

signals:

public slots:
    void connection();
    void removeThread(QString filename);
};

#endif // SERVER_H