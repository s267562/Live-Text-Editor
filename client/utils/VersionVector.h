// Created by simone on 07/08/19.

#ifndef TEXTEDITOR_VERSIONVECTOR_H
#define TEXTEDITOR_VERSIONVECTOR_H

#include "Version.h"
#include <vector>

/**
 * vector/list of versions of sites in the distributed system
 * keeps track of the latest operation received from each site (i.e. version)
 * prevents duplicate operations from being applied to our CRDT
 */

class VersionVector {
public:
    VersionVector(QString siteId = {});

    const Version &getLocalVersion() const;
    const std::vector<Version> &getVersions() const;
    void increment();

private:
    Version localVersion;
    std::vector<Version> versions;
    void update(Version incomingVersion);
};


#endif //TEXTEDITOR_VERSIONVECTOR_H
