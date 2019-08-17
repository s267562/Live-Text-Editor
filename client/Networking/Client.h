#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <queue>
#include "../utils/Constants.h"
#include "../utils/Identifier.h"
#include "../utils/Character.h"
#include "message/Message.h"

class Client: public QObject{
    Q_OBJECT
private:
    QTcpSocket *socket;
    bool reciveOkMessage;
    bool clientIsLogged;
    std::queue<QByteArray> messages;
    std::queue<Message> incomingMessagesQueue;

public:
    Client(QObject *parent = nullptr);
    bool writeOnSocket(QString);
    void insert(QString, QString, int pos);
    /*void insert(QString, std::vector<Identifier> pos);                        // Testing
    void insert(QString str, std::vector<int> pos);                             // Testing
    void deleteChar(QString, int pos);*/                                        // Testing
    void deleteChar(QString str, QString siteId,std::vector<int> pos);          // Testing
    void deleteChar(QString str, QString siteId, std::vector<Identifier> pos);
    bool writeOnSocket(std::string str);                                        // Testing
    bool connectTo(QString host);
    void logIn(QString username, QString passsword);
    void logOut();
    void registration(QString username, QString passsword, QString pathAvatar);
    void requestForFile(QString fileName);
    bool readInsert();
    bool readDelete();

public slots:
    void onReadyRead();
    void onDisconnect();
};

#endif // CLIENT_H
