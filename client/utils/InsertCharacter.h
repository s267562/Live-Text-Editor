//
// Created by andrea settimo on 2019-09-06.
//

#ifndef TEXTEDITOR_INSERTCHARACTER_H
#define TEXTEDITOR_INSERTCHARACTER_H


#include <QString>
#include <QJsonObject>
#include "CharFormat.h"
#include "Pos.h"

class InsertCharacter {
    char character;
    QString siteId;
    CharFormat charFormat;
    Pos pos;

public:
    InsertCharacter(char character, const QString &siteId, const CharFormat &charFormat, const Pos &pos) :
                                            character(character),
                                            siteId(siteId),
                                            charFormat(charFormat),
                                            pos(pos) {}

    InsertCharacter():pos(0,0){}

    void write(QJsonObject &json) const {
        json["character"] = character;
        json["siteId"] = siteId;

        QJsonObject charFormatObject;
        charFormat.write(charFormatObject);
        json["charFormat"] = charFormatObject;

        QJsonObject posObject;
        pos.write(posObject);
        json["pos"] = posObject;
    }

    void read(const QJsonObject &json) {
        if (json.contains("character") && json["character"].isDouble())
                character = static_cast<char>(json["character"].toInt());

        if (json.contains("charFormat"))
                charFormat.read(json["charFormat"].toObject());

        if (json.contains("siteId") && json["siteId"].isString())
                siteId = json["siteId"].toString();

        if (json.contains("pos"))
                pos.read(json["pos"].toObject());
    }

    QByteArray toQByteArray(){
        QJsonObject json;
        this->write(json);
        QJsonDocument document(json);
        return document.toBinaryData();
    }

    static InsertCharacter toInsertCharacter(QJsonDocument jsonDocument){
        QJsonObject jsonObject = jsonDocument.object();
        InsertCharacter character;
        character.read(jsonObject);
        return character;
    }

    char getCharacter() const {
            return character;
    }

    const QString &getSiteId() const {
            return siteId;
    }

    const CharFormat &getCharFormat() const {
            return charFormat;
    }

    const Pos &getPos() const {
            return pos;
    }
};


#endif //TEXTEDITOR_INSERTCHARACTER_H
