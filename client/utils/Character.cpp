//
// Created by simone on 06/08/19.
//

#include "Character.h"

Character::Character(char value, int counter, const QString &siteId, const std::vector<Identifier> &position)
        : value(value), counter(counter), siteId(siteId), position(position) {}

int Character::compareTo(Character otherCharacter) {
    std::vector<Identifier> pos1 = this->getPosition();
    std::vector<Identifier> pos2 = otherCharacter.getPosition();

    for (int i = 0; i < std::min(pos1.size(), pos2.size()); i++) {
        Identifier id1 = pos1[i];
        Identifier id2 = pos2[i];
        int comp = id1.compareTo(id2);

        if (comp != 0) {
            return comp;
        }
    }

    if (pos1.size() < pos2.size()) {
        return -1;
    } else if (pos1.size() > pos2.size()) {
        return 1;
    } else {
        return 0;
    }
}

char Character::getValue() const {
    return value;
}

int Character::getCounter() const {
    return counter;
}

const QString &Character::getSiteId() const {
    return siteId;
}

const std::vector<Identifier> &Character::getPosition() const {
    return position;
}
