//
// Created by simone on 06/08/19.
//

#include "Identifier.h"

Identifier::Identifier(int digit, const std::string &siteId) : digit(digit), siteId(siteId) {}

int Identifier::getDigit() const {
    return digit;
}

const std::string &Identifier::getSiteId() const {
    return siteId;
}
