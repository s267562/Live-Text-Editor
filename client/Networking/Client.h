#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <queue>
#include "../utils/Constants.h"

class Client: public QObject{
    Q_OBJECT
private:
    QTcpSocket *socket;
    bool reciveOkMessage;
    bool clientIsLogged;
    std::queue<QByteArray> messages;
public:
    Client(QObject *parent = nullptr);
    bool writeOnSocket(QString);
    void insert(QString,int pos);
    void deleteChar(QString,int pos);
    bool writeOnSocket(std::string str);
    bool connectTo(QString host);
    void logIn(QString username, QString passsword);
    void requestForFile(QString fileName);

public slots:
    void onReadyRead();
    void onDisconnect();
};

#endif // CLIENT_H
