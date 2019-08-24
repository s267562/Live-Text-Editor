//
// Created by Eugenio Marinelli on 2019-08-21.
//

#ifndef TEXTEDITOR_MAINWINDOW_H
#define TEXTEDITOR_MAINWINDOW_H


#include <QtWidgets/QMainWindow>
#include "Editor.h"
#include "ui/login.h"
#include "Networking/Client.h"
#include "ui/connection.h"
#include "ui/showFiles.h"

class MainWindow : public QMainWindow {

Q_OBJECT

public:
    MainWindow(QString siteId);
    void show();

private:
    Editor *editor;
    Login *login;
    Client *client;
    CRDT *crdt;
    Controller *controller;
    Connection *connection;
    Registration *registration;
    ShowFiles *finder;

private slots:
    void showEditor();
    void connectClient(QString address);
    void showLogin();
    void showRegistration();
    void showFileFinder();
    void requestForFile(QString);

    signals:
        void loginSuccessful();
};


#endif //TEXTEDITOR_MAINWINDOW_H
