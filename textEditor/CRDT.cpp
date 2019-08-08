//
// Created by simone on 05/08/19.
//

#include "CRDT.h"
#include <math.h>
#include <iostream>


CRDT::CRDT(std::string siteId) : siteId(siteId), vector(siteId) {
    this->structure = { { } }; // TODO inizializzare il vettore structure?
}

/**
 * User inserts character into their local text editor and sends the operation to all other users.
 * The only information needed is the character value and the editor index at which it is inserted.
 * A new character object will then be created using that information and inserted into the CRDT array.
 * Finally, the new character is returned so it can be sent to the other users.
 *
 * @param c
 * @param index
 * @return
 */
void CRDT::localInsert(std::vector<char> chars, Pos startPos) {
    for(int i = 0; i < chars.size(); i++) {
        this->handleLocalInsert(chars[i], startPos);

        startPos.incrementCh();
        if(chars[i] == '\n') {
            startPos.incrementLine();
            startPos.resetCh();
        }
    }
}

void CRDT::handleLocalInsert(char val, Pos pos) {
    this->vector.increment();
    const Character character = generateChar(val, pos);
    insertChar(character, pos);
    std::cout << val << " inserted." << std::endl;

    std::cout << "-----------------------" << std::endl << "STRUCTURE:" << std::endl;
    for(int i=0; i<structure.size(); i++) {
        for(int j=0; j<structure[i].size(); j++) {
            char val = structure[i][j].getValue();
            int counter = structure[i][j].getCounter();
            std::cout << "val = " << val << "; counter = " << counter << "; position: ";
            std::vector<Identifier> identifier = structure[i][j].getPosition();
            for(Identifier id : identifier) {
                std::cout << id.getDigit() << " ";
            }
            std::cout << std::endl;
        }
    }

    // TODO this->controller.broadcastInsertion(char);
}


const Character CRDT::generateChar(char val, Pos pos) {
    const std::vector<Identifier> posBefore = findPosBefore(pos);
    const std::vector<Identifier> posAfter = findPosAfter(pos);
    const std::vector<Identifier> newPos = generatePosBetween(posBefore, posAfter);

    Character character(val, this->vector.getLocalVersion().getCounter(), siteId, newPos);

    return character;
}

const std::vector<Identifier> CRDT::findPosBefore(Pos pos) {
    int ch = pos.getCh();
    int line = pos.getLine();

    if (ch == 0 && line == 0) {
        return std::vector<Identifier> {};
    } else if (ch == 0 && line != 0) {
        line = line - 1;
        ch = structure[line].size();
    }

    return structure[line][ch - 1].getPosition();
}

const std::vector<Identifier> CRDT::findPosAfter(Pos pos) {
    int ch = pos.getCh();
    int line = pos.getLine();

    int numLines = structure.size();
    int numChars = 0;
    if(line < numLines) numChars = structure[line].size();

    if (line == numLines - 1 && ch == numChars) {
        return std::vector<Identifier> {};
    } else if (line < numLines - 1 && ch == numChars) {
        line = line + 1;
        ch = 0;
    } else if (line > numLines - 1 && ch == 0) {
        return std::vector<Identifier> {};
    }

    return structure[line][ch].getPosition();
}

std::vector<Identifier> CRDT::generatePosBetween(std::vector<Identifier> pos1, std::vector<Identifier> pos2, std::vector<Identifier> newPos, int level) {

    // change 2 to any other number to change base multiplication
    int base = pow(2, level) * CRDT::base;
    // TODO? char boundaryStrategy = this.retrieveStrategy(level);

    Identifier id1{0, this->siteId}, id2{base, this->siteId};
    if(pos1.size() > 0 ) id1 = pos1[0];
    if(pos2.size() > 0 ) id2 = pos2[0];

    if (id2.getDigit() - id1.getDigit() > 1) {
        int newDigit = this->generateIdBetween(id1.getDigit(), id2.getDigit());
        Identifier newId{newDigit, this->siteId};
        newPos.push_back(newId);
        return newPos;
    } else if (id2.getDigit() - id1.getDigit() == 1) {
        newPos.push_back(id1);

        std::vector<Identifier> pos1_2;
        if(pos1.size() > 0) pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
        return this->generatePosBetween(pos1_2, std::vector<Identifier>{}, newPos, level + 1);

    } else /* if (id1.getDigit() == id2.getDigit()) */ {
        if (id1.getSiteId() < id2.getSiteId()) {
            newPos.push_back(id1);
            std::vector<Identifier> pos1_2;
            if(pos1.size() > 0) pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
            return this->generatePosBetween(pos1_2, std::vector<Identifier>{}, newPos, level + 1);
            return this->generatePosBetween(std::vector<Identifier>(pos1.begin() + 1, pos1.end()), std::vector<Identifier>{}, newPos, level + 1);
        } else /* if (id1.getSiteId() == id2.getSiteId()) */ {
            newPos.push_back(id1);
            std::vector<Identifier> pos1_2, pos2_2;
            if(pos1.size() > 0) pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
            if(pos2.size() > 0) pos2_2 = std::vector<Identifier>(pos2.begin() + 1, pos2.end());
            return this->generatePosBetween(pos1_2, pos2_2, newPos, level + 1);
        } /* else {
            // throw new Error("Fix Position Sorting"); // TODO capire quando può capitare questo caso e come gestirlo.
        } */
    }
}

int CRDT::generateIdBetween(int min, int max) {
    int boundary = 10;
    if (max - min < boundary) {
        min = min + 1;
    } else {
        min = min + 1;
        max = min + boundary;
    }
    srand (time(0)); // seed for different random value
    return (int)floor(((double) rand() / (RAND_MAX)) * (max - min)) + min; // ((double) rand() / (RAND_MAX)) -> random value [0, 1)
}

void CRDT::insertChar(Character character, Pos pos) {

    if (pos.getLine() == structure.size()) {
        structure.push_back(std::vector<Character> {}); // pushing a new line.
    }

    // if inserting a newline, split line into two lines.
    if (character.getValue() == '\n') {
        std::vector<Character> lineAfter(structure[pos.getLine()].begin() + pos.getCh(), structure[pos.getLine()].end()); // get line after.
        if (lineAfter.size() != 0) {
            structure.insert(structure.begin() + pos.getLine() + 1, lineAfter); // insert line after.
        }
    }
    structure[pos.getLine()].insert(structure[pos.getLine()].begin() + pos.getCh(), character); // insert the character in the pos.
}


void CRDT::localDelete(Pos startPos, Pos endPos) {
    // TODO
}