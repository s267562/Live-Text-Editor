//
// Created by simone on 05/08/19.
//

#ifndef TEXTEDITOR_CRDT_H
#define TEXTEDITOR_CRDT_H

#include <string>
#include <vector>
#include "utils/Pos.h"
#include "utils/Character.h"
#include "utils/Identifier.h"
#include "utils/VersionVector.h"

class CRDT {
public:
    CRDT(std::string siteId);
    void localInsert(std::vector<char> chars, Pos startPos);
    void localDelete(Pos startPos, Pos endPos);

private:
    std::string siteId;
    std::vector<std::vector<Character>> structure;
    VersionVector vector;
    static const int base = 32;

    void handleLocalInsert(char val, Pos pos);
    int remoteInsert(Character character);
    int remoteDelete(char);

    const Character generateChar(char val, Pos pos);
    const std::vector<Identifier> findPosBefore(Pos pos);
    const std::vector<Identifier> findPosAfter(Pos pos);
    std::vector<Identifier> generatePosBetween(std::vector<Identifier> pos1, std::vector<Identifier> pos2, std::vector<Identifier> newPos = std::vector<Identifier>{}, int level = 0);
    int generateIdBetween(int min, int max);
    void insertChar(Character character, Pos pos);
};


#endif //TEXTEDITOR_CRDT_H
