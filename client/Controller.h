//
// Created by simone on 09/08/19.
//

#ifndef TEXTEDITOR_CONTROLLER_H
#define TEXTEDITOR_CONTROLLER_H

#include "CRDT.h"
#include "Editor.h"
#include <iostream>

class Editor;

class Controller {
private:
    CRDT *crdt;
    Editor *editor;
    std::string siteId;

public:
    Controller(CRDT *crdt, Editor *editor);
    void localInsert(std::vector<char> chars, Pos startPos);
    void localDelete(Pos startPos, Pos endPos);
    // TODO handleRemoteOperation -> remoteInsert, remoteDelete

};


#endif //TEXTEDITOR_CONTROLLER_H
