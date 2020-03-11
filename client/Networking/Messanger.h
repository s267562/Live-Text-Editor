#ifndef MESSANGER_H
#define MESSANGER_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <queue>
#include "../../common/Constants.h"
#include "../utils/Identifier.h"
#include "../utils/Character.h"
#include "../utils/Pos.h"
#include "message/Message.h"
#include "../CRDT/CRDT.h"
#include "../User/User.h"
#include "../../common/commonFunctions.h"

class CRDT;

class Messanger: public QObject{
    Q_OBJECT
private:
    QString serverIP;
    QString serverPort;
    QTcpSocket *socket;
    qintptr socketDescriptor;
    SocketState state;
    bool reciveOkMessage;
    bool clientIsLogged;
    std::queue<QByteArray> messages;
    CRDT *crdt = nullptr;
    bool clientIsDisconnected;

private:

    QMetaObject::Connection connectReadyRead;
    QMetaObject::Connection connectDisconnected;

public:
    Messanger(QObject *parent = nullptr);
    /*bool writeInsert(Character character);
    bool writeStyleChanged(Character character);
    bool writeDelete(Character character);*/
    //bool writeDelete(Character character);
    bool writeAlignmentChanged(int alignment_type, Character blockId); // gestire con un segnale forse
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
    bool requestForUsernameList(QString fileName);
    bool readUsernameList();
    bool sendFileInfomationChanges(QString oldFilename, QString newFilename, QStringList usernames);
    bool sendDeleteFile(QString filename);
    void setCrdt(CRDT *crdt);

    bool readAlignmentChanged();


    User *user = nullptr;

public slots:
    void onReadyRead();
    void onDisconnect();
    bool logOut();
    bool writeInsert(Character character);
    bool writeStyleChanged(Character character);
    bool writeDelete(Character character);

signals:
    void errorConnection();
    void loginFailed();
    void registrationFailed();
    void newMessage(Message message);
    void fileNames(std::map<QString, bool> fileList);
    void logout();
    void setUsers(QStringList);
    void removeUser(QString);
    void fileRecive(std::vector<std::vector<Character>> file, std::vector<std::pair<Character,int>> at);
    void requestForFileFailed();
    void insertFailed();
    void deleteFailed();
    void reciveUser(User *user);
    void editAccountFailed();
    void okEditAccount();
    void shareCodeFailed();
    void addFileNames(std::map<QString, bool> fileList);
    void reciveUsernameList(QString filename, QStringList userlist);
};

#endif // MESSANGER_H
