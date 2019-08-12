//
// Created by simone on 06/08/19.
//

#include "Character.h"

Character::Character(char value, int counter, const std::string &siteId, const std::vector<Identifier> &position)
        : value(value), counter(counter), siteId(siteId), position(position) {}

char Character::getValue() const {
    return value;
}

int Character::getCounter() const {
    return counter;
}

const std::string &Character::getSiteId() const {
    return siteId;
}

const std::vector<Identifier> &Character::getPosition() const {
    return position;
}
