//
// Created by simone on 05/08/19.
//

#ifndef TEXTEDITOR_CRDT_H
#define TEXTEDITOR_CRDT_H

#include <string>
#include <vector>
#include "../client/utils/Character.h"
#include "../client/utils/Identifier.h"
#include "../client/utils/VersionVector.h"
#include "../client/utils/Pos.h"
#include <math.h>
#include <iostream>

class CRDT {
public:
    CRDT();

    Character handleInsert(char val, Pos pos, QString siteId);
    void handleDelete(const Character &character);


private:
    QString siteId;
    std::vector<std::vector<Character>> structure;
    VersionVector vector;
    static const int base = 32;

    // insert
    const Character generateChar(char val, Pos pos);
    const std::vector<Identifier> findPosBefore(Pos pos);
    const std::vector<Identifier> findPosAfter(Pos pos);
    std::vector<Identifier> generatePosBetween(std::vector<Identifier> pos1, std::vector<Identifier> pos2, std::vector<Identifier> newPos = {}, int level = 0);
    int generateIdBetween(int min, int max);
    void insertChar(Character character, Pos pos);

    // delete
    Pos findPosition(Character character);
    int findIndexInLine(Character character, std::vector<Character> line);
    void removeEmptyLines();
    void mergeLines(int line);
};


#endif //TEXTEDITOR_CRDT_H
