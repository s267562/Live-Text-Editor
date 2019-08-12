//
// Created by simone on 07/08/19.
//

#include "Version.h"

Version::Version(std::string siteId) :siteId(siteId), counter() { }

int Version::getCounter() const {
    return counter;
}

const std::string &Version::getSiteId() const {
    return siteId;
}

void Version::increment() {
    this->counter++;
}