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


class Editor;
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
    User *user;
    QString siteId;
    bool requestFFile = false;

public:
    /* VIEWS */
    Editor *editor = nullptr;
    Login *login = nullptr;
    Connection *connection = nullptr;
    Registration *registration = nullptr;
    ShowFiles *showFiles = nullptr;
    QWidget *now;
    QMainWindow *GUI = new QMainWindow(this);

    /* NETWORKING */
    Messanger *messanger;

    Loading *loading = nullptr;
    bool loadingPoPupIsenabled = false;
    CustomWidget *customWidget = nullptr;

    /* Connection */
    void networkingConnection();
    void crdtConnection();
    void loginConnection();
    void loginDisconnection();
    void registrationConnection();
    void registrationDisconnection();
    void finderConnection();
    void finderDisconnection();
    void editorConnection();
    void editorDisconnection();

    void handleGUI(QMainWindow *window);

public:
    Controller(CRDT *crdt, Editor *editor, Messanger *messanger);
    Controller();
    User* getUser();
    void startLoadingPopup();
    void stopLoadingPopup();
    void sendFileInformationChanges(QString oldFileaname, QString newFileaname, const QStringList& usernames);
    void sendDeleteFile(QString filename);
    CRDT *getCrdt() const;
    bool isRequestFFile() const;
    void setRequestFFile(bool requestFFile);
    QMainWindow *getGui() const;
    Messanger *getMessanger() const;
    void handleError();

public slots:
    /* ------------------------------------------------------------- NETWORKING */
    void errorConnection();
    void timeout();

    /* ------------------------------------------------------------- CONNECTION */
    void connectClient(QString address, QString port);

    /*  ------------------------------------------------------------- LOGIN */
    void showLogin();

    /* ------------------------------------------------------------- REGISTRATION */
    void showRegistration();

    /* ------------------------------------------------------------- SHOW FILES */
    void showFileFinder(const QMap<QString, bool>&);
    void showFileFinderOtherView();
    void requestForFile(const QString& filename);
    void addFileNames(QMap<QString, bool> filenames);
    void requestForUsernameList(QString filename, CustomWidget *customWideget);
    void reciveUsernameList(const QString& filename, QStringList userlist);

    /* ------------------------------------------------------------- EDITOR */
    void showEditor();
    void openFile(const std::vector<std::vector<Character>>& initialStructure, std::vector<std::pair<Character,int>> styleBlocks, QString filename);
    void reciveUser(User *user);
    void sendEditAccount(QString username, QString newPassword, QString oldPassword, const QByteArray& avatar);
    void errorEditAccount();
    void okEditAccount();
    void sendShareCode(const QString& sharecode);
    void shareCodeFailed();
    void reciveExternalErrorOrException();
    void inviledateTextEditor();

signals:
    void userRecived();
    void reset();

    void resetCrdt();
};


#endif //TEXTEDITOR_CONTROLLER_H
