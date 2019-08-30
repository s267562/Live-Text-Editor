//
// Created by simone on 09/08/19.
//

#include "Controller.h"

Controller::Controller(CRDT *crdt, Editor *editor, Client *client) : crdt(crdt), editor(editor), client(client) {
    editor->setController(this);

    // Controller
    connect(client, &Client::newMessage,
              this, &Controller::newMessage);

    // TO-DO: connect(client, &Client::newMessage, this, &Controller::/* metodo che vuoi richimare della classe */);
}

void Controller::localInsert(QString chars, Pos startPos) {
    // send insert at the server. To insert it in the model we need the position computed by the server.
    this->client->insert(chars, this->siteId, startPos);
}

void Controller::localDelete(Pos startPos, Pos endPos) {
    std::vector<Character> removedChars = this->crdt->handleDelete(startPos, endPos);

    for(Character c : removedChars) {
        this->client->deleteChar(QString{c.getValue()}, this->siteId, c.getPosition());
    }
}

void Controller::newMessage(Message message) {
    // Message message = this->client->getMessage();

    if(message.getType() == INSERT) {
        Character character = message.getCharacter();
        Pos pos = this->crdt->insert(character);

        if(character.getSiteId().compare(this->siteId) == 0) { // if QStrings are equal it returns 0
            // local insert - only in the model; the char is already in the view.
        } else {
            // remote insert - the char is to insert in the model and in the view.
            // insert into the editor.
            this->editor->insertChar(character.getValue(), pos);
        }
    } else if(message.getType() == DELETE) {
        if(!(message.getCharacter().getSiteId() == this->siteId)) {
            Pos pos = this->crdt->handleRemoteDelete(message.getCharacter());

            if(pos) {
                // delete from the editor.
                this->editor->deleteChar(pos);
            }
        }
    }
}

void Controller::resetModel() {
    this->crdt->reset();
    this->client->resetModel(this->siteId);
}
