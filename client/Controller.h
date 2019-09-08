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
#include <iostream>

class Editor;

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

public slots:
    /* NETWORKING */
    void errorConnection();

    /* CONNECTION */
    void connectClient(QString address);

    /* LOGIN */
    void showLogin();

    /* REGISTRATION */
    void showRegistration();

    /* SHOW FILES */
    void showFileFinder(QStringList);
    void showFileFinderOtherView();
    void requestForFile(QString filename);

    /* EDITOR */
    void showEditor();
    void newMessage(Message message);
    void openFile(std::vector<std::vector<Character>> initialStructure);

public:
    Controller(CRDT *crdt, Editor *editor, Messanger *messanger);
    Controller();
    void localInsert(QString val, CharFormat charFormat, Pos pos);
    void localDelete(Pos startPos, Pos endPos);
    void resetModel();
};


#endif //TEXTEDITOR_CONTROLLER_H
