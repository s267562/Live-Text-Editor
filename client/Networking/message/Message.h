//
// Created by andrea settimo on 2019-08-17.
//

#ifndef TEXTEDITOR_MESSAGE_H
#define TEXTEDITOR_MESSAGE_H

#include <QtGlobal>
#include "../../../client/utils/Character.h"

enum TypeMessage {INSERT,DELETE,STYLE_CHANGED,ALIGNMENT_CHANGED};

class Message {
    Character character;
    qintptr socketID;
    TypeMessage type;

    alignment_type at;
    int blockNumber;
    /* adding timestamp? */
public:
    Message(Character character, qintptr socketID, TypeMessage type, alignment_type at=LEFT, int blockNumber=-1):
    character(character), socketID(socketID), type(type), at(at), blockNumber(blockNumber) {}


    const Character &getCharacter() const {
        return character;
    }

    qintptr getSocketID() const {
        return socketID;
    }

    TypeMessage getType() const {
        return type;
    }

    int getBlockNumber()const {
        return blockNumber;
    }
    alignment_type getAlignmentType()const {
        return at;
    }

};


#endif //TEXTEDITOR_MESSAGE_H
