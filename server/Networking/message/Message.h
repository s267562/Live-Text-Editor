//
// Created by andrea settimo on 2019-08-17.
//

#ifndef TEXTEDITOR_MESSAGE_H
#define TEXTEDITOR_MESSAGE_H

#include <QtGlobal>
#include "../../../client/utils/Character.h"

enum TypeMessage {
    INSERT,
    DELETE,
    RESET};

class Message {
    Character character;
    qintptr socketID;
    TypeMessage type;
    /* adding timestamp? */
public:
    Message(Character character, qintptr socketID, TypeMessage type): character(character), socketID(socketID), type(type){}


    const Character &getCharacter() const {
        return character;
    }

    qintptr getSocketID() const {
        return socketID;
    }

    TypeMessage getType() const {
        return type;
    }


};


#endif //TEXTEDITOR_MESSAGE_H
