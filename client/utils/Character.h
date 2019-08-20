//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_CHARACTER_H
#define TEXTEDITOR_CHARACTER_H

#include <string>
#include <vector>
#include <QtCore/QString>
#include "Identifier.h"

class Character {
public:
    Character(char value, int counter, const QString &siteId, const std::vector<Identifier> &position);

    char getValue() const;
    int getCounter() const;
    const QString &getSiteId() const;
    const std::vector<Identifier> &getPosition() const;

    int compareTo(Character otherCharacter);

private:
    char value;
    int counter;
    QString siteId;
    std::vector<Identifier> position;
};


#endif //TEXTEDITOR_CHARACTER_H
