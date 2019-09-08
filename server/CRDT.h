//
// Created by simone on 05/08/19.
//

#ifndef TEXTEDITOR_CRDT_H
#define TEXTEDITOR_CRDT_H

#include <string>
#include <vector>
#include "../client/utils/Character.h"
#include "../client/utils/Identifier.h"
#include "../client/utils/Pos.h"
#include <math.h>
#include <iostream>

class CRDT {
public:
    CRDT();

    Pos handleInsert(Character character);
    void handleDelete(const Character &character);
	bool loadCRDT(QString filename);
	bool saveCRDT(QString filename);
	const std::vector<std::vector<Character>> &getStructure() const;

private:
    std::vector<std::vector<Character>> structure;
	std::map<QString, int> versionsVector; // map<socketDescriptor, counter>
    static const int base = 32;


    // insert
    Pos findInsertPosition(Character character);
    Pos findEndPosition(Character lastChar, std::vector<Character> lastLine, int totalLines);
    void insertChar(Character character, Pos pos);


    // delete
    Pos findPosition(Character character);
    int findIndexInLine(Character character, std::vector<Character> line);
    void removeEmptyLines();
    void mergeLines(int line);

	void read(const QJsonObject &json);
	void write( QJsonObject &json) const;
};


#endif //TEXTEDITOR_CRDT_H
