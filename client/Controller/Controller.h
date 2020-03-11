//
// Created by simone on 09/08/19.
//

#ifndef TEXTEDITOR_CONTROLLER_H
#define TEXTEDITOR_CONTROLLER_H

#include "../CRDT/CRDT.h"
#include "../Editor/editor.h"
#include "../User/User.h"
#include "Login/login.h"
#include "Connection/connection.h"
#include "ShowFiles/showFiles.h"
#include "../Networking/Messanger.h"
#include "Loading/loading.h"
#include <iostream>
#include "CustomWidget/customwidget.h"
#include "../CRDT/CDRTThread.h"

class Editor;
class CDRTThread;
class ShowFiles;
class Login;
class Registration;
class CustomWidget;
class CRDT;
class Messanger;

class Controller : public QMainWindow {
    Q_OBJECT

    //TODO: Add #ifdef
    friend class TestGui;
private:
    /* model */
    CRDT *crdt;
    CDRTThread *crdtThread;
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
    CustomWidget *customWidget = nullptr;

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
    void openFile(std::vector<std::vector<Character>> initialStructure, std::vector<std::pair<Character,int>> styleBlocks);

    void reciveUser(User *user);
    void sendEditAccount(QString username, QString newPassword, QString oldPassword, QByteArray avatar);
    void errorEditAccount();
    void okEditAccount();
    void sendShareCode(QString sharecode);
    void shareCodeFailed();
    void alignChange(int alignment_type, int blockNumber);
    void requestForUsernameList(QString filename, CustomWidget *customWideget);
    void reciveUsernameList(QString filename, QStringList userlist);

    /*void localInsert(QString val, QTextCharFormat textCharFormat, Pos pos); ->in CRDT
    void totalLocalInsert(int charsAdded, QTextCursor cursor, QString chars,  int position);
    void localDelete(Pos startPos, Pos endPos);*/

signals:
    void userRecived();
    /* MULTI THREAD */
    void insertChar(char character, QTextCharFormat charFormat, Pos pos);
    void changeStyle(Pos pos, const QTextCharFormat&format);
    void deleteChar(Pos pos);
    void reset();

public:
    Controller(CRDT *crdt, Editor *editor, Messanger *messanger);
    Controller();

    User* getUser();
    //void styleChange(QTextCharFormat textCharFormat, Pos pos);  ->in CRDT
    void startLoadingPopup();
    void stopLoadingPopup();
    void sendFileInformationChanges(QString oldFileaname, QString newFileaname, QStringList usernames);
    void sendDeleteFile(QString filename);
    CRDT *getCrdt() const;
    ~Controller();
};


#endif //TEXTEDITOR_CONTROLLER_H
