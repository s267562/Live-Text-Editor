//
// Created by simone on 09/08/19.
//

#ifndef TEXTEDITOR_CONTROLLER_H
#define TEXTEDITOR_CONTROLLER_H

#include "CRDT.h"
#include "editor.h"
#include "User.h"
#include "ui/login.h"
#include "ui/connection.h"
#include "ui/showFiles.h"
#include "Networking/Messanger.h"
#include "ui/loading.h"
#include <iostream>

class Editor;
class ShowFiles;
class Login;
class Registration;

class Controller : public QMainWindow {
    Q_OBJECT
private:
    /* model */
    CRDT *crdt;
    User *user;
    QString siteId;

    /* view */
    Editor *editor;
    Login *login;
    Connection *connection;
    Registration *registration;
    ShowFiles *finder;
    QWidget *now;

    /* networking */
    Messanger *messanger;
    Loading *loading = nullptr;

public slots:
    /* NETWORKING */
    void errorConnection();

    /* CONNECTION */
    void connectClient(QString address, QString port);

    /* LOGIN */
    void showLogin();

    /* REGISTRATION */
    void showRegistration();

    /* SHOW FILES */
    void showFileFinder(std::map<QString, bool>);
    void showFileFinderOtherView();
    void requestForFile(QString filename);
    void addFileNames(std::map<QString, bool> filenames);

    /* EDITOR */
    void showEditor();
    void newMessage(Message message);
    void openFile(std::vector<std::vector<Character>> initialStructure);

    void reciveUser(User *user);
    void sendEditAccount(QString username, QString newPassword, QString oldPassword, QByteArray avatar);
    void errorEditAccount();
    void okEditAccount();
    void sendShareCode(QString sharecode);
    void shareCodeFailed();

public:
    Controller(CRDT *crdt, Editor *editor, Messanger *messanger);
    Controller();
    void localInsert(QString val, QTextCharFormat textCharFormat, Pos pos);
    void localDelete(Pos startPos, Pos endPos);
    User* getUser();
    void styleChange(QTextCharFormat textCharFormat, Pos pos);
    void startLoadingPopup();
    void stopLoadingPopup();
};


#endif //TEXTEDITOR_CONTROLLER_H
