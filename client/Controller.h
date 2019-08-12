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
class CRDT;

class Controller {
private:
    CRDT *crdt;
    Editor *editor;
    std::string siteId;
    Client *client;

public:
    Controller(CRDT *crdt, Editor *editor, Client *client);
    void localInsert(std::vector<char> chars, Pos startPos);
    void localDelete(Pos startPos, Pos endPos);
    Client* getClient();
    // TODO handleRemoteOperation -> remoteInsert, remoteDelete

};


#endif //TEXTEDITOR_CONTROLLER_H
