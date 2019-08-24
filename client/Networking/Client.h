#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <queue>
#include "../utils/Constants.h"
#include "../utils/Identifier.h"
#include "../utils/Character.h"
#include "../utils/Pos.h"
#include "message/Message.h"

class Client: public QObject{
    Q_OBJECT
private:
    QTcpSocket *socket;
    qintptr socketDescriptor;
    bool reciveOkMessage;
    bool clientIsLogged;

private:
    std::queue<QByteArray> messages;
    std::queue<Message> incomingInsertMessagesQueue;
    std::queue<Message> incomingDeleteMessagesQueue;

public:
    Client(QObject *parent = nullptr);
    bool writeOnSocket(QString);
    void insert(QString, QString, Pos pos);
    /*void insert(QString, std::vector<Identifier> pos);                        // Testing
    void insert(QString str, std::vector<int> pos);                             // Testing
    void deleteChar(QString, int pos);*/                                        // Testing
    void deleteChar(QString str, QString siteId,std::vector<int> pos);          // Testing
    void deleteChar(QString str, QString siteId, std::vector<Identifier> pos);
    bool writeOnSocket(std::string str);                                        // Testing
    bool connectTo(QString host);
    bool logIn(QString username, QString passsword);
    void logOut();
    void registration(QString username, QString passsword, QString pathAvatar);
    bool requestForFile(QString fileName);
    bool readInsert();
    bool readDelete();



    Message getMessage();

public slots:
    void onReadyRead();
    void onDisconnect();

signals:
    void errorConnection();
    void loginFailed();
    void newMessage();
    void userVerified();
};

#endif // CLIENT_H
