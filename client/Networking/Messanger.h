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
#include "../User.h"

class Messanger: public QObject{
    Q_OBJECT
private:
    SocketState state;
    QString siteId;
    QString serverIP;
    QString serverPort;
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
    bool writeInsert(Character character);
    bool writeStyleChanged(Character character);
    bool writeDelete(Character character);
    bool writeAlignmentChanged(int alignment_type, int blockNumber);
    bool connectTo(QString host, QString port);    
    bool logIn(QString username, QString passsword);
    bool readFileNames();
    bool registration(QString username, QString password, QByteArray avatar);
    bool requestForFile(QString fileName);
    bool readInsert();
    bool readStyleChanged();
    bool readDelete();
    bool readUsernames();
    bool readRemoveUser();
    bool readFile();
    bool despatchMessage();
    bool readError();
    bool readUser();
    bool sendEditAccount(QString username, QString newPassword, QString oldPassword, QByteArray avatar);
    bool sendShareCode(QString shareCode);
    bool readAddFile();

    bool readAlignmentChanged();


    User *user = nullptr;

public slots:
    void onReadyRead();
    void onDisconnect();
    bool logOut();

signals:
    void errorConnection();
    void loginFailed();
    void registrationFailed();
    void newMessage(Message message);
    void userVerified();
    void fileNames(std::map<QString, bool> fileList);
    void logout();
    void setUsers(QStringList);
    void removeUser(QString);
    void fileRecive(std::vector<std::vector<Character>> file, std::vector<int > at);
    void requestForFileFailed();
    void insertFailed();
    void deleteFailed();
    void reciveUser(User *user);
    void editAccountFailed();
    void okEditAccount();
    void shareCodeFailed();
    void addFileNames(std::map<QString, bool> fileList);


};

#endif // MESSANGER_H
