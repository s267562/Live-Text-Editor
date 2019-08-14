#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <queue>
#include "../utils/Constants.h"
#include "../utils/Identifier.h"

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
    void insert(QString, QString, int pos);
    /*void insert(QString, std::vector<Identifier> pos);
    void insert(QString str, std::vector<int> pos);*/
    void deleteChar(QString,int pos);
    void deleteChar(QString str, std::vector<int> pos);
    void deleteChar(QString str, std::vector<Identifier> pos);
    bool writeOnSocket(std::string str);
    bool connectTo(QString host);
    void logIn(QString username, QString passsword);
    void logOut();
    void registration(QString username, QString passsword, QString pathAvatar);
    void requestForFile(QString fileName);
    bool readInsert();

public slots:
    void onReadyRead();
    void onDisconnect();
};

#endif // CLIENT_H
