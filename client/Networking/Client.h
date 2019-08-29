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
    void deleteChar(QString str, QString siteId, std::vector<Identifier> pos);
    bool connectTo(QString host);
    bool logIn(QString username, QString passsword);
    bool readFileNames();
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
    void newMessage(Message message);
    void userVerified();
    void fileNames(QStringList fileList);
};

#endif // CLIENT_H
