//
// Created by andrea settimo on 2020-03-18.
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
#include <shared_mutex>


class Editor;
class CDRTThread;
class ShowFiles;
class Login;
class Registration;
class CustomWidget;
class CRDT;
class Messanger;
class OtherCursor;

class Controller : public QMainWindow {
    Q_OBJECT

    //TODO: Add #ifdef
    friend class TestGui;
private:
    /* MODEL */
    CRDT *crdt;
    CDRTThread *crdtThread;
    User *user;
    QString siteId;
    bool requestFFile = false;
public:
    bool isRequestFFile() const;

    void setRequestFFile(bool requestFFile);

private:

    /* VIEWS */
    Editor *editor = nullptr;
    Login *login = nullptr;
    Connection *connection = nullptr;
    Registration *registration = nullptr;
    ShowFiles *finder = nullptr;
    QWidget *now;
    QMainWindow *GUI = new QMainWindow(this);
public:
    QMainWindow *getGui() const;

private:

    /* NETWORKING */
    Messanger *messanger;
public:
    Messanger *getMessanger() const;

private:
    Loading *loading = nullptr;
    bool loadingPoPupIsenabled = false;
    CustomWidget *customWidget = nullptr;

    /* Connection */
    void networkingConnection();
    void crdtConnection();
    void loginConnection();
    void registrationConnection();
    void registrationDisconnection();
    void finderConnection();
    void finderDisconnection();
    void editorConnection();

    void handleGUI(QMainWindow *window);

public:
    std::shared_mutex mutexRequestForFile;

    Controller(CRDT *crdt, Editor *editor, Messanger *messanger);
    Controller();
    User* getUser();
    void startLoadingPopup();
    void stopLoadingPopup();
    void sendFileInformationChanges(QString oldFileaname, QString newFileaname, QStringList usernames);
    void sendDeleteFile(QString filename);
    CRDT *getCrdt() const;
    ~Controller();

public slots:
    /* ------------------------------------------------------------- NETWORKING */
    void errorConnection();

    /* ------------------------------------------------------------- CONNECTION */
    void connectClient(QString address, QString port);

    /*  ------------------------------------------------------------- LOGIN */
    void showLogin();

    /* ------------------------------------------------------------- REGISTRATION */
    void showRegistration();

    /* ------------------------------------------------------------- SHOW FILES */
    void showFileFinder(std::map<QString, bool>);
    void showFileFinderOtherView();
    void requestForFile(QString filename);
    void addFileNames(std::map<QString, bool> filenames);
    void requestForUsernameList(QString filename, CustomWidget *customWideget);
    void reciveUsernameList(QString filename, QStringList userlist);

    /* ------------------------------------------------------------- EDITOR */
    void showEditor();
    void openFile(std::vector<std::vector<Character>> initialStructure, std::vector<std::pair<Character,int>> styleBlocks, QString filename);
    void reciveUser(User *user);
    void sendEditAccount(QString username, QString newPassword, QString oldPassword, QByteArray avatar);
    void errorEditAccount();
    void okEditAccount();
    void sendShareCode(QString sharecode);
    void shareCodeFailed();
    //void alignChange(int alignment_type, int blockNumber);

signals:
    void userRecived();
    void reset();
};


#endif //TEXTEDITOR_CONTROLLER_H
