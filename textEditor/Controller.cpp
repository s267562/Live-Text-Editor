//
// Created by simone on 09/08/19.
//

#include "Controller.h"

Controller::Controller(CRDT *crdt, Editor *editor) : crdt(crdt), editor(editor) {
    editor->setController(this);
    crdt->setController(this);
}

/**
 * User inserts character into their local text editor and sends the operation to all other users.
 * The only information needed is the character value and the editor index at which it is inserted.
 * A new character object will then be created using that information and inserted into the CRDT array.
 * Finally, the new character is returned so it can be sent to the other users.
 *
 * @param c
 * @param index
 * @return
 */
void Controller::localInsert(std::vector<char> chars, Pos startPos) {
    for(int i = 0; i < chars.size(); i++) {
        this->crdt->handleLocalInsert(chars[i], startPos);

        startPos.incrementCh();
        if(chars[i] == '\n') {
            startPos.incrementLine();
            startPos.resetCh();
        }
    }
}

void Controller::localDelete(Pos startPos, Pos endPos) {
    this->crdt->handleLocalDelete(startPos, endPos);
}