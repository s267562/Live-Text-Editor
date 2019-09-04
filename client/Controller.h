//
// Created by simone on 09/08/19.
//

#ifndef TEXTEDITOR_CONTROLLER_H
#define TEXTEDITOR_CONTROLLER_H

#include "CRDT.h"
#include "editor.h"
#include "Networking/Client.h"
#include <iostream>

class Editor;

class Controller : public QObject {
    Q_OBJECT
private:
    CRDT *crdt;
    Editor *editor;
    Client *client;

private slots:
    void newMessage(Message message);

public:
    Controller(CRDT *crdt, Editor *editor, Client *client);
    void localInsert(QString chars, Pos startPos);
    void localDelete(Pos startPos, Pos endPos);
    void resetModel();
    void openFile(std::vector<std::vector<Character>> structure);
};


#endif //TEXTEDITOR_CONTROLLER_H
