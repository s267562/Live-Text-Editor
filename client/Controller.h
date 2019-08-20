//
// Created by simone on 09/08/19.
//

#ifndef TEXTEDITOR_CONTROLLER_H
#define TEXTEDITOR_CONTROLLER_H

#include "CRDT.h"
#include "Editor.h"
#include "Networking/Client.h"
#include <iostream>

class Editor;

class Controller : public QObject {
private:
    CRDT *crdt;
    Editor *editor;
    Client *client;
    QString siteId;

private slots:
    void newMessage();

public:
    Controller(CRDT *crdt, Editor *editor, Client *client);
    void localInsert(QString chars, Pos startPos);
    void localDelete(Pos startPos, Pos endPos);
    void insertIntoEditor(char character, Pos pos);

};


#endif //TEXTEDITOR_CONTROLLER_H
