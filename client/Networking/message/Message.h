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

    int alignment;
    QString sender;
    /* adding timestamp? */
public:
    Message(Character character, qintptr socketID, TypeMessage type, QString sender, int alignment=0x1):
    character(character), socketID(socketID), type(type), alignment(alignment), sender(sender) {}


    const Character &getCharacter() const {
        return character;
    }

    qintptr getSocketID() const {
        return socketID;
    }

    TypeMessage getType() const {
        return type;
    }

    int getAlignmentType()const {
        return alignment;
    }

    const QString &getSender() const {
        return sender;
    }

};


#endif //TEXTEDITOR_MESSAGE_H
