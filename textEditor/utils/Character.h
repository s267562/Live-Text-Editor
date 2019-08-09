//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_CHARACTER_H
#define TEXTEDITOR_CHARACTER_H

#include <string>
#include <vector>
#include "Identifier.h"

class Character {
public:
    Character(char value, int counter, const std::string &siteId, const std::vector<Identifier> &position);

    char getValue() const;
    int getCounter() const;
    const std::string &getSiteId() const;
    const std::vector<Identifier> &getPosition() const;

private:
    char value;
    int counter;
    std::string siteId;
    std::vector<Identifier> position;
};


#endif //TEXTEDITOR_CHARACTER_H
