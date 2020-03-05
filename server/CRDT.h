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
#include "Utils/Constants.h"
#include <math.h>
#include <iostream>
#include <QtCharts>

//enum align {LEFT, CENTER, RIGHT, JUSTIFY};

class CRDT {
public:
    CRDT();

    Pos handleInsert(Character character);
    Pos handleStyleChanged(const Character &character);
    void handleDelete(const Character &character);
	bool loadCRDT(QString filename);
	bool saveCRDT(QString filename);
	const std::vector<std::vector<Character>> &getStructure() const;

	void handleAlignmentChanged(int alignment, int blockNumber);
	void insertIntoTextDocument(char character, QTextCharFormat charFormat, Pos pos);
	void deleteFromTextDocument(Pos pos);
	void changeStyleOfDocument(Pos pos, const QTextCharFormat &textCharFormat);
	//void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

	QTextDocument* getTextDocument();
	int getRow(Character blockId);

private:
    std::vector<std::vector<Character>> structure;
	std::map<QString, int> versionsVector; // map<socketDescriptor, counter>
    static const int base = 32;
	std::vector<std::pair<Character,int>> style;

	QTextDocument *td;
	QTextCursor textCursor;

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

    void removeStyleLine(int i);
};


#endif //TEXTEDITOR_CRDT_H
