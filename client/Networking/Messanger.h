#ifndef MESSANGER_H
#define MESSANGER_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <queue>
#include "../utils/Constants.h"
#include "../utils/Identifier.h"
#include "../utils/Character.h"
#include "../utils/Pos.h"
#include "message/Message.h"
#include "../CRDT.h"

class Messanger: public QObject{
    Q_OBJECT
private:
    QString siteId;
    QTcpSocket *socket;
    qintptr socketDescriptor;
    CRDT *crdt;
    bool reciveOkMessage;
    bool clientIsLogged;
    QMetaObject::Connection c;
    QMetaObject::Connection d;

private:
    std::queue<QByteArray> messages;

public:
    Messanger(QObject *parent = nullptr);
    void setCRDT(CRDT *crdt);
    bool writeOnSocket(QString);
    bool insert(QString, CharFormat charFormat, Pos pos);
    bool deleteChar(QString str, std::vector<Identifier> pos);
    bool connectTo(QString host);
    bool logIn(QString username, QString passsword);
    bool readFileNames();
    bool registration(QString username, QString password, QPixmap avatar);
    bool requestForFile(QString fileName);
    bool readInsert();
    bool readDelete();
    bool readUsernames();
    bool readRemoveUser();
    bool readFile();
    bool despatchMessage();

public slots:
    void onReadyRead();
    void onDisconnect();
    void logOut();

signals:
    void errorConnection();
    void loginFailed();
    void newMessage(Message message);
    void userVerified();
    void fileNames(QStringList fileList);
    void logout();
    void setUsers(QStringList);
    void removeUser(QString);
    void fileRecive(std::vector<std::vector<Character>> file);
};

#endif // MESSANGER_H
