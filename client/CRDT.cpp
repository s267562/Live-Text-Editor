//
// Created by simone on 05/08/19.
//

#include <ctime>
#include <cmath>
#include "CRDT.h"
#include <QDebug>


CRDT::CRDT() {
    this->structure = {};
}

void CRDT::setStructure(const std::vector<std::vector<Character>> &initialStructure) {
    if(initialStructure.empty()) {
        CRDT::structure = {};
    } else {
        CRDT::structure = initialStructure;
    }
}

const QString &CRDT::getSiteId() const {
    return siteId;
}

void CRDT::setSiteId(const QString &siteId) {
    CRDT::siteId = siteId;
}

const Character CRDT::getCharacter(Pos pos) {
    
    int line=pos.getLine();
    int ch=pos.getCh();
    
    if( line>=0 && line<this->structure.size() ){ // TODO: Extend this check to the server too
        
        if( ch>=0 && ch<this->structure[line].size() ){
            return this->structure[line][ch];
        }
    }
    return Character(); // Default character
}

// remote insert

Pos CRDT::handleRemoteInsert(Character character) {

    Pos pos = this->findInsertPosition(character);
    this->insertChar(character, pos);
    qDebug() << "\nclient/CRDT.cpp - handleRemoteInsert():";
    printStructures();
    // print the structure for debugging
//    qDebug() << "client/CRDT.cpp - handleRemoteInsert()     ---------- STRUCTURE ----------";
//    for (int i = 0; i < structure.size(); i++) {
//        for (int j = 0; j < structure[i].size(); j++) {
//            QDebug qD(QtDebugMsg);
//            char val = structure[i][j].getValue();
//            int counter = structure[i][j].getCounter();
//            QString siteId = structure[i][j].getSiteId();
//            QString value = ""; if(val == '\n') value += "\n"; else value += val;
//            if(i == pos.getLine() && j == pos.getCh()) {
//                qD << "                                ---> val:" << value << "  siteId: " << siteId << "  counter:" << counter << "  position:";
//            } else {
//                qD << "                                     val:" << value << "  siteId: " << siteId << "  counter:" << counter << "  position:";
//            }
//            std::vector<Identifier> identifier = structure[i][j].getPosition();
//            for (Identifier id : identifier) {
//                qD << id.getDigit();
//            }
//        }
//    }
//    qDebug() << ""; // newLine

    return pos;
}

Pos CRDT::findInsertPosition(Character character) {
    // check if struct is empty or char is less than first char
    if (this->structure.empty() || character.compareTo(this->structure[0][0]) < 0) {
        return Pos {0, 0}; // false obj
    }

    int minLine = 0;
    int totalLines = this->structure.size();
    int maxLine = totalLines - 1;
    std::vector<Character> lastLine = this->structure[maxLine];

    Character lastChar = lastLine[lastLine.size() - 1];

    // char is greater than all existing chars (insert at end)
    if (character.compareTo(lastChar) > 0) {
        return this->findEndPosition(lastChar, lastLine, totalLines);
    }

    // binary search
    while (minLine + 1 < maxLine) {
        int midLine = std::floor(minLine + (maxLine - minLine) / 2);
        std::vector<Character> currentLine = this->structure[midLine];
        lastChar = currentLine[currentLine.size() - 1];

        if (character.compareTo(lastChar) == 0) {
            return Pos{ (int) currentLine.size() - 1, midLine };
        } else if (character.compareTo(lastChar) < 0) {
            maxLine = midLine;
        } else {
            minLine = midLine;
        }
    }

    // Check between min and max line.
    std::vector<Character> minCurrentLine = this->structure[minLine];
    Character minLastChar = minCurrentLine[minCurrentLine.size() - 1];
    std::vector<Character> maxCurrentLine = this->structure[maxLine];
    Character maxLastChar = maxCurrentLine[maxCurrentLine.size() - 1];

    if (character.compareTo(minLastChar) <= 0) {
        int charIdx = this->findIndexInLine(character, minCurrentLine);
        return Pos { charIdx, minLine };
    } else {
        int charIdx = this->findIndexInLine(character, maxCurrentLine);
        return Pos { charIdx, maxLine };
    }

}

Pos CRDT::findEndPosition(Character lastChar, std::vector<Character> lastLine, int totalLines) {
    if (lastChar.getValue() == '\n') {
        return Pos { 0, totalLines };
    } else {
        return Pos { (int) lastLine.size(), totalLines - 1 };
    }
}



// local insert

Character CRDT::handleLocalInsert(char val, QTextCharFormat textCharFormat, Pos pos) {
    //increment version vector
    this->versionsVector[siteId]++;

    const Character character = generateChar(val, textCharFormat, pos, siteId);
    insertChar(character, pos);

    /*if(this->structure[pos.getLine()].size()==1) { // line==first char + \n
        this->style.insert(this->style.begin()+pos.getLine(), std::pair<Character,int>(character, 0x4));
    }*/

    //qDebug() << "server/CRDT.cpp - handleInsert()     " << val << " inserted.";

    // print the structure for debugging
    qDebug() << "\nclient/CRDT.cpp - handleInsert():";
    printStructures();

    return character;
}

const Character CRDT::generateChar(char val, QTextCharFormat textCharFormat, Pos pos, QString siteId) {
    const std::vector<Identifier> posBefore = findPosBefore(pos);
    const std::vector<Identifier> posAfter = findPosAfter(pos);
    const std::vector<Identifier> newPos = generatePosBetween(posBefore, posAfter, siteId);

    Character character(val, textCharFormat, this->versionsVector[siteId], siteId, newPos);

    return character;
}

const std::vector<Identifier> CRDT::findPosBefore(Pos pos) {
    int ch = pos.getCh();
    int line = pos.getLine();

    if (ch == 0 && line == 0) {
        return std::vector<Identifier>{};
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
    if (line < numLines) numChars = structure[line].size();

    if (line == numLines - 1 && ch == numChars) {
        return std::vector<Identifier>{};
    } else if (line < numLines - 1 && ch == numChars) {
        line = line + 1;
        ch = 0;
    } else if (line > numLines - 1 && ch == 0) {
        return std::vector<Identifier>{};
    }

    return structure[line][ch].getPosition();
}

std::vector<Identifier> CRDT::generatePosBetween(std::vector<Identifier> pos1, std::vector<Identifier> pos2, QString siteId, std::vector<Identifier> newPos, int level) {

    // change 2 to any other number to change base multiplication
    int base = pow(2, level) * CRDT::base;
    // TODO? char boundaryStrategy = this.retrieveStrategy(level);

    Identifier id1{0, siteId}, id2{base, siteId};
    if (pos1.size() > 0) id1 = pos1[0];
    if (pos2.size() > 0) id2 = pos2[0];

    if (id2.getDigit() - id1.getDigit() > 1) {
        int newDigit = this->generateIdBetween(id1.getDigit(), id2.getDigit());
        Identifier newId{newDigit, siteId};
        newPos.push_back(newId);
        return newPos;
    } else if (id2.getDigit() - id1.getDigit() == 1) {
        newPos.push_back(id1);

        std::vector<Identifier> pos1_2;
        if (pos1.size() > 0) pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
        return this->generatePosBetween(pos1_2, std::vector<Identifier>{}, siteId, newPos, level + 1);

    } else /* if (id1.getDigit() == id2.getDigit()) */ {
        if (id1.getSiteId() < id2.getSiteId()) {
            // TODO check if correct...
            newPos.push_back(id1);
            std::vector<Identifier> pos1_2;
            pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
            return this->generatePosBetween(pos1_2, std::vector<Identifier>{}, siteId, newPos, level + 1);
        } else /* if (id1.getSiteId() == id2.getSiteId()) */ {
            // TODO check if correct...
            newPos.push_back(id1);
            std::vector<Identifier> pos1_2, pos2_2;
            if (pos1.size() > 0) pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
            if (pos2.size() > 0) pos2_2 = std::vector<Identifier>(pos2.begin() + 1, pos2.end());
            return this->generatePosBetween(pos1_2, pos2_2, siteId, newPos, level + 1);
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
    srand(time(0)); // seed for different random value
    return (int) floor(((double) rand() / (RAND_MAX)) * (max - min)) +
           min; // ((double) rand() / (RAND_MAX)) -> random value [0, 1)
}


// common function for model insertion
void CRDT::insertChar(Character character, Pos pos) {

    qDebug() << "Char: " << character.getValue() << "inserted in pos " << pos.getLine() << pos.getCh();

    if (pos.getLine() == structure.size()) {
        structure.push_back(std::vector<Character> {}); // pushing a new line.
        style.push_back(std::pair<Character,int>(character,0x4));
    }

    // if inserting a newline, split line into two lines.
    if (character.getValue() == '\n') {
        qDebug() << "Splitting line into two lines";
        std::vector<Character> lineAfter(structure[pos.getLine()].begin() + pos.getCh(), structure[pos.getLine()].end()); // get line after.
        if (lineAfter.size() != 0) {
            qDebug().noquote() << "There is something after the newLine inserted";
            structure[pos.getLine()].erase(structure[pos.getLine()].begin() + pos.getCh(), structure[pos.getLine()].end()); // delete line after.

            /*
            if(structure.size() <= pos.getLine()+1) {
                // the line + 1 does not exist.
                qDebug() << "line + 1 does not exist. Pushing back.";
                structure.push_back(lineAfter); // pushing back the line after.
            } else {
                // the line + 1 exists.
                qDebug() << "line + 1 exists, Inserting.";
                structure.insert(structure.begin() + pos.getLine() + 1, lineAfter.begin(), lineAfter.end());
            }
             */
            structure.insert(structure.begin() + pos.getLine() + 1, lineAfter);
            this->style.insert(this->style.begin()+pos.getLine()+1, std::pair<Character,int>(character,0x4));

        } else {
            qDebug().noquote() << "There is nothing after the char \n inserted";
            //this->style.insert(this->style.begin()+pos.getLine(),std::pair<Character,int>(character,0x4));

        }
    }

    structure[pos.getLine()].insert(structure[pos.getLine()].begin() + pos.getCh(), character); // insert the character in the pos.
}



// local delete

std::vector<Character> CRDT::handleLocalDelete(Pos startPos, Pos endPos) {
    bool newlineRemoved = false;
    std::vector<Character> removedChars;

    // for multi-line deletes
    if (startPos.getLine() != endPos.getLine()) {

        // delete chars on first line from startPos.ch to end of line
        newlineRemoved = true;
        removedChars = this->deleteMultipleLines(startPos, endPos);

        // single-line deletes
    } else {
        removedChars = this->deleteSingleLine(startPos, endPos);

        for(Character c : removedChars) {
            if(c.getValue() == '\n') newlineRemoved = true;
        }
    }

    this->removeEmptyLines();

    if (newlineRemoved && startPos.getCh() > 0) {
        //qDebug() << "need to merge line" << startPos.getLine();
        this->mergeLines(startPos.getLine());
    }

    // print the structure for debugging
    qDebug() << "\nclient/CRDT.cpp - handleDelete(): \n";

    this->printStructures();

    return removedChars;
}

std::vector<Character> CRDT::deleteMultipleLines(Pos startPos, Pos endPos) {
    std::vector<Character> chars = {};

    if (this->structure[endPos.getLine()].size() > 0) {
        chars.insert(chars.end(), structure[endPos.getLine()].begin(), structure[endPos.getLine()].begin() + endPos.getCh());
        structure[endPos.getLine()].erase(structure[endPos.getLine()].begin(), structure[endPos.getLine()].begin() + endPos.getCh());
    }

    for (int line = endPos.getLine() - 1; line > startPos.getLine(); line--) {
        chars.insert(chars.end(), structure[line].begin(), structure[line].end());
        structure.erase(structure.begin() + line );
        style.erase(style.begin() + line );
    }

    chars.insert(chars.end(), structure[startPos.getLine()].begin() + startPos.getCh(), structure[startPos.getLine()].end());
    structure[startPos.getLine()].erase(structure[startPos.getLine()].begin() + startPos.getCh(), structure[startPos.getLine()].end());

    return chars;
}

std::vector<Character> CRDT::deleteSingleLine(Pos startPos, Pos endPos) {
    // TODO check if correct
    int charNum = endPos.getCh() - startPos.getCh();
    //qDebug() << "client/CRDT.cpp - deleteSingleLine()     charNum: " << charNum;
    //qDebug() << "client/CRDT.cpp - deleteSingleLine()     startPos.getCh(): " << startPos.getCh();
    //qDebug() << "client/CRDT.cpp - deleteSingleLine()     startPos.getCh() + charNum: " << startPos.getCh() + charNum;
    //qDebug() << "client/CRDT.cpp - deleteSingleLine()     structure[startPos.getLine()].size(): " << structure[startPos.getLine()].size();
    if(structure[startPos.getLine()].size() < startPos.getCh() + charNum) {
        // TODO lanciare un'eccezione per evitare crash?
        qDebug() << "client/CRDT.cpp - deleteSingleLine()     ATTENZIONE: impossibile cancellare. Char/s non presente/i";
        qDebug() << ""; // newLine
    }
    std::vector<Character> chars {structure[startPos.getLine()].begin() + startPos.getCh(), structure[startPos.getLine()].begin() + startPos.getCh() + charNum};
    this->structure[startPos.getLine()].erase(structure[startPos.getLine()].begin() + startPos.getCh(), structure[startPos.getLine()].begin() + startPos.getCh() + charNum);

    return chars;
}


// remote delete

Pos CRDT::handleRemoteDelete(const Character &character) {
    Pos pos = this->findPosition(character);

    if (!pos) return pos;

    this->structure[pos.getLine()].erase(this->structure[pos.getLine()].begin() + pos.getCh());

    if (character.getValue() == '\n') {
        this->mergeLines(pos.getLine());
    }

    this->removeEmptyLines();

    // print the structure for debugging
    qDebug() << "\nclient/CRDT.cpp - handleRemoteDelete(): ";
    printStructures();
    return pos;
}

Pos CRDT::findPosition(Character character) {
    // check if struct is empty or char is less than first char
    if (this->structure.empty() || character.compareTo(this->structure[0][0]) < 0) {
        return Pos {-1, -1}; // false obj
    }

    int minLine = 0;
    int totalLines = this->structure.size();
    int maxLine = totalLines - 1;
    std::vector<Character> lastLine = this->structure[maxLine];

    Character lastChar = lastLine[lastLine.size() - 1];

    // char is greater than all existing chars (insert at end)
    if (character.compareTo(lastChar) > 0) {
        return Pos {-1, -1}; // false obj
    }

    // binary search
    while (minLine + 1 < maxLine) {
        int midLine = std::floor(minLine + (maxLine - minLine) / 2);
        std::vector<Character> currentLine = this->structure[midLine];
        lastChar = currentLine[currentLine.size() - 1];

        if (character.compareTo(lastChar) == 0) {
            return Pos { (int) currentLine.size() - 1, midLine };
        } else if (character.compareTo(lastChar) < 0) {
            maxLine = midLine;
        } else {
            minLine = midLine;
        }
    }

    // Check between min and max line.
    std::vector<Character> minCurrentLine = this->structure[minLine];
    Character minLastChar = minCurrentLine[minCurrentLine.size() - 1];
    std::vector<Character> maxCurrentLine = this->structure[maxLine];
    Character maxLastChar = maxCurrentLine[maxCurrentLine.size() - 1];


    if (character.compareTo(minLastChar) <= 0) {
        int charIdx = this->findIndexInLine(character, minCurrentLine);
        return Pos { charIdx, minLine };
    } else {
        int charIdx = this->findIndexInLine(character, maxCurrentLine);
        return Pos { charIdx, maxLine };
    }
}

void CRDT::removeEmptyLines() {
    for (int line = 0; line < this->structure.size(); line++) {
        if (this->structure[line].size() == 0) {
            this->structure.erase(this->structure.begin() + line);
            this->style.erase(this->style.begin()+line);
            line--;
            
        }
    }
}

void CRDT::mergeLines(int line) {
    if(structure.size() > line + 1 && structure[line + 1].size() > 0) {
        structure[line].insert(structure[line].end(), structure[line + 1].begin(), structure[line + 1].end());
        //qDebug() << "EREASING line" << line + 1 << " line size:" << structure[line+1].size();
        structure.erase(structure.begin() + line + 1);
        style.erase(style.begin()+line+1);
    }
}


// common function for remoteInsert and remoteDelete
int CRDT::findIndexInLine(Character character, std::vector<Character> line) {

    int left = 0;
    int right = line.size() - 1;

    if (line.size() == 0 || character.compareTo(line[left]) < 0) {
        return left;
    } else if (character.compareTo(line[right]) > 0) {
        return this->structure.size();
    }

    while (left + 1 < right) {
        int mid = std::floor(left + (right - left) / 2);
        int compareNum = character.compareTo(line[mid]);

        if (compareNum == 0) {
            return mid;
        } else if (compareNum > 0) {
            left = mid;
        } else {
            right = mid;
        }
    }

    if (character.compareTo(line[left]) == 0) {
        return left;
    } else {
        return right;
    }

}



const QString CRDT::toText() {
    QString text = "";
    for(int i=0; i<structure.size(); i++) {
        for(int j=0; j<structure[i].size(); j++) {
            text += structure[i][j].getValue();
        }
    }
    return text;
}

// local style changed
bool CRDT::styleChanged(QTextCharFormat textCharFormat, Pos pos) {
    if(structure[pos.getLine()][pos.getCh()].getTextCharFormat() == textCharFormat) {
        return false;
    } else {
        structure[pos.getLine()][pos.getCh()].setTextCharFormat(textCharFormat);
        return true;
    }
}


// remote style changed
Pos CRDT::handleRemoteStyleChanged(const Character &character) {
    Pos pos = findPosition(character);

    this->structure[pos.getLine()][pos.getCh()].setTextCharFormat(character.getTextCharFormat());

    return pos;
}

void CRDT::insertBlock(Character character, Pos position) {

   /* if( position.getLine()==this->style.size() ) {
        this->style.push_back(std::pair<Character, int>());
    }*/
    if( position.getCh()==0 ){
        Pos blockPos(0,position.getLine());

        this->style.insert(
                this->style.begin()+position.getLine(),std::pair<Character,int>(generateChar('-',character.getTextCharFormat(),blockPos,character.getSiteId()),
                                                                                0x4));
    }



    if (character.getValue() == '\n') {
        Pos blockPos(0,position.getLine()+1);
        this->style.insert(
                this->style.begin()+position.getLine(),std::pair<Character,int>(generateChar('-',character.getTextCharFormat(),blockPos,character.getSiteId()), 0x4));
    }




}

Character CRDT::getBlockIdentifier(int blockNumber) {
    return this->style[blockNumber].first;
}


int CRDT::getRow(Character blockId) {
    if (this->style.empty() || blockId.compareTo(this->style[0].first) < 0) {
        return -1;
    }

    int minLine = 0;
    int totalLines = this->structure.size();
    int maxLine = totalLines - 1;

    Character lastBlockId = style[maxLine].first;

    // char is greater than all existing chars (insert at end)
    if (blockId.compareTo(lastBlockId) > 0) {
        return -1;
    }

    // binary search
    while (minLine + 1 < maxLine) {
        int midLine = std::floor(minLine + (maxLine - minLine) / 2);

        lastBlockId = style[midLine].first;

        if (blockId.compareTo(lastBlockId) == 0) {
            return midLine;
        } else if (blockId.compareTo(lastBlockId) < 0) {
            maxLine = midLine;
        } else {
            minLine = midLine;
        }
    }

    return -1;
}


void CRDT::printStructures() {

    QDebug qD(QtDebugMsg);
    qD << "\t\t\t\t\t\t---------- STRUCTURE ----------\n";


    for (int i = 0; i < structure.size(); i++) {
        QString value="";
        char val_line = style[i].first.getValue();

        if(val_line == '\n') {
            value += "\n";
        }else {
            value += val_line;
        }

        qD << "\n\tLINE ID:\tvalue:" << value << "\tposition: ";
        std::vector<Identifier> line = style[i].first.getPosition();
        for (Identifier id : line) {
            qD << id.getDigit();
        }
        qD << "\n";
        qD << "\t\tCHARACTERS:\n";
        for (int j = 0; j < structure[i].size(); j++) {

            QString value="";

            char val_char = structure[i][j].getValue();

            int counter = structure[i][j].getCounter();

            QString siteId = structure[i][j].getSiteId();
            if(val_char == '\n') value += "\n"; else value += val_char;
            qD << "\t\t\tval: " << value << "\t\tsiteId: " << siteId << "\t\tcounter: " << counter << "\t\tposition: ";
            std::vector<Identifier> identifier = structure[i][j].getPosition();
            for (Identifier id : identifier) {
                qD << id.getDigit();
            }
            qD << "\n";
        }
    }

    qD << "\nNumber of rows in STYLE: "<< this->style.size() << "\tNumber of rows in STRUCTURE: "<< this->structure.size() <<"\n\n"; // newLine

}

