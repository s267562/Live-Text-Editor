//
// Created by simone on 07/08/19.
//

#include "VersionVector.h"

VersionVector::VersionVector(std::string siteId) : localVersion(siteId) {
    this->versions = std::vector<Version> {};
    this->versions.push_back(this->localVersion);
}

const Version &VersionVector::getLocalVersion() const {
    return localVersion;
}

const std::vector<Version> &VersionVector::getVersions() const {
    return versions;
}

void VersionVector::increment() {
    this->localVersion.increment();
}

/*
 * updates vector with new version received from another site if vector doesn't contain version,
 * it's created and added to vector create exceptions if need be.
 */
void VersionVector::update(Version incomingVersion) {
    // TODO ?
}

// TODO metodi hasBeenApplied, getVersionFromVector ?