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
#include <math.h>
#include <iostream>

class CRDT {
public:
    CRDT(QString siteId);

    std::vector<Character> handleDelete(Pos startPos, Pos endPos);
    Pos insert(Character character);
    Pos handleRemoteDelete(const Character &character);
    void reset();

private:
    QString siteId;
    std::vector<std::vector<Character>> structure;
    VersionVector vector;
    static const int base = 32;

    // insert
    Pos findInsertPosition(Character character);
    Pos findEndPosition(Character lastChar, std::vector<Character> lastLine, int totalLines);
    int findInsertIndexInLine(Character character, std::vector<Character> line);
    void insertChar(Character character, Pos pos);

    // delete
    Pos findPosition(Character character);
    int findIndexInLine(Character character, std::vector<Character> line);


    std::vector<Character> deleteMultipleLines(Pos startPos, Pos endPos);
    std::vector<Character> deleteSingleLine(Pos startPos, Pos endPos);
    void removeEmptyLines();

    void mergeLines(int line);

};


#endif //TEXTEDITOR_CRDT_H
