//
// Created by simone on 05/08/19.
//

#include <ctime>
#include <QString>
#include "CRDT.h"

CRDT::CRDT(std::string siteId) : siteId(siteId), vector(siteId) {
    this->structure = { { } }; // TODO inizializzare il vettore structure?
}

void CRDT::setController(Controller *controller) {
    CRDT::controller = controller;
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
    controller->getClient()->insert(QString(character.getValue()),character.getPosition());
}

void CRDT::handleLocalDelete(Pos startPos, Pos endPos) {
    bool newlineRemoved = false;

    // for multi-line deletes
    if (startPos.getLine() != endPos.getLine()) {
        // delete chars on first line from startPos.ch to end of line
        newlineRemoved = true;
        std::vector<Character> chars = this->deleteMultipleLines(startPos, endPos);

        // single-line deletes
    } else {
        std::vector<Character> chars = this->deleteSingleLine(startPos, endPos);

        for(Character c : chars) {
            if(c.getValue() == '\n') newlineRemoved = true;
        }
    }

    this->removeEmptyLines();

    if (newlineRemoved && this->structure[startPos.getLine() + 1].size() > 0) {
        this->mergeLines(startPos.getLine());
    }
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



std::vector<Character> CRDT::deleteMultipleLines(Pos startPos, Pos endPos) {
    // TODO check if correct
    std::vector<Character> chars {structure[startPos.getLine()].begin(), structure[startPos.getLine()].begin() + startPos.getCh()};
    structure[startPos.getLine()].erase(structure[startPos.getLine()].begin() + startPos.getCh());

    for (int line = startPos.getLine() + 1; line < endPos.getLine(); line++) {
        chars.insert(chars.end(), structure[line + 1].begin(), structure[line + 1].end());
    }

    // to do for loop inside crdt
    if (this->structure[endPos.getLine()].size() > 0) {
        std::vector<Character> vec { this->structure[endPos.getLine()].begin(), this->structure[endPos.getLine()].begin() + endPos.getCh() };
        chars.insert(chars.end(), vec.begin(), vec.end());
    }

    return chars;

}

std::vector<Character> CRDT::deleteSingleLine(Pos startPos, Pos endPos) {
    // TODO check if correct
    int charNum = endPos.getCh() - startPos.getCh();
    std::vector<Character> chars {structure[startPos.getLine()].begin() + startPos.getCh(), structure[startPos.getLine()].begin() + startPos.getCh() + charNum};
    this->structure[startPos.getLine()].erase(structure[startPos.getLine()].begin() + startPos.getCh(), structure[startPos.getLine()].begin() + startPos.getCh() + charNum);

    return chars;
}

void CRDT::removeEmptyLines() {
    // TODO check if correct
    for (int line = 0; line < this->structure.size(); line++) {
        if (this->structure[line].size() == 0) {
            this->structure.erase(this->structure.begin() + line);
            line--;
        }
    }

    if (this->structure.size() == 0) {
        this->structure.push_back(std::vector<Character>{});
    }
}

void CRDT::mergeLines(int line) {
    // TODO check if correct
    if(structure.size() > line + 1 && structure[line + 1].size() > 0) {
        structure[line].insert(structure[line].end(), structure[line + 1].begin(), structure[line + 1].end());
    }
}