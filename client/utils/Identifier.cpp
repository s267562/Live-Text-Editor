//
// Created by simone on 06/08/19.
//

#include "Identifier.h"

Identifier::Identifier(int digit, const QString &siteId) : digit(digit), siteId(siteId) {}

int Identifier::getDigit() const {
    return digit;
}

const QString &Identifier::getSiteId() const {
    return siteId;
}

int Identifier::compareTo(Identifier otherIdentifier) {
    if (this->getDigit() < otherIdentifier.getDigit()) {
        return -1;
    } else if (this->getDigit() > otherIdentifier.getDigit()) {
        return 1;
    } else {
        if (this->getSiteId() < otherIdentifier.getSiteId()) {
            return -1;
        } else if (this->getSiteId() > otherIdentifier.getSiteId()) {
            return 1;
        } else {
            return 0;
        }
    }
}