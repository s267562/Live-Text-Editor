//
// Created by simone on 07/08/19.
//

#include "Version.h"

Version::Version(QString siteId) : siteId(siteId), counter() { }

int Version::getCounter() const {
    return counter;
}

const QString &Version::getSiteId() const {
    return siteId;
}

void Version::increment() {
    this->counter++;
}