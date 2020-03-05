//
// Created by simone on 05/08/19.
//

#include <ctime>
#include <cmath>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include "CRDT.h"
#include <QDebug>

CRDT::CRDT() : td(new QTextDocument), textCursor(td) {
    this->style.emplace_back(std::pair<Character,int> {});
}

const std::vector<std::vector<Character>> &CRDT::getStructure() const {
    return structure;
}

QTextDocument* CRDT::getTextDocument(){
    return this->td;
}

// handle insert

Pos CRDT::handleInsert(Character character) {

    Pos pos = this->findInsertPosition(character);
    this->insertChar(character, pos);
    this->insertIntoTextDocument(character.getValue(),character.getTextCharFormat(),pos);

    // print the structure for debugging
    QDebug qDPos(QtDebugMsg);
    qDPos << "BLOCK VECTOR SIZE: " << this->style.size() << "  STRUCTURE VECTOR SIZE: " << this->structure.size();

    for (int i = 0; i < structure.size(); i++) {



        qDPos << "\n\n BLOCK: " << this->style[i].first.getValue() << "  ";

        std::for_each(this->style[i].first.getPosition().begin(), this->style[i].first.getPosition().end(),[&qDPos](Identifier i){
            qDPos<<i.getDigit();
        });


        for (int j = 0; j < structure[i].size(); j++) {
            QDebug qD(QtDebugMsg);
            char val = structure[i][j].getValue();
            int counter = structure[i][j].getCounter();
            QString siteId = structure[i][j].getSiteId();
            QString value = ""; if(val == '\n') value += "\n"; else value += val;
            if(i == pos.getLine() && j == pos.getCh()) {
                qD << "                                ---> val:" << value << "  siteId: " << siteId << "  counter:" << counter << "  position:";
            } else {
                qD << "                                     val:" << value << "  siteId: " << siteId << "  counter:" << counter << "  position:";
            }
            std::vector<Identifier> identifier = structure[i][j].getPosition();
            for (Identifier id : identifier) {
                qD << id.getDigit();
                qD << " ";
            }
        }
    }
    qDebug() << ""; // newLine

    //auto pt=td->toPlainText();
    //auto ss=pt.toStdString();
    //qDebug() << td->toHtml();

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
            return Pos{ (int) currentLine.size() - 1,midLine };
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

void CRDT::insertChar(Character character, Pos pos) {

    qDebug() << "Char: " << character.getValue() << "inserted in pos " << pos.getLine() << pos.getCh();

    if( structure.empty() || pos.getLine() == structure.size()){
        structure.emplace_back(std::vector<Character> {}); // pushing a new line.
    }
    

    // if inserting a newline, split line into two lines.
    if (character.getValue() == '\n') {
        qDebug() << "Splitting line into two lines";
        std::vector<Character> lineAfter(structure[pos.getLine()].begin() + pos.getCh(), structure[pos.getLine()].end()); // get line after.
        if (lineAfter.size() != 0) {
            qDebug().noquote() << "There is something after the newLine inserted";
            structure[pos.getLine()].erase(structure[pos.getLine()].begin() + pos.getCh(), structure[pos.getLine()].end()); // delete line after.
            structure.insert(structure.begin() + pos.getLine() + 1, lineAfter);
            this->style.insert(this->style.begin() + pos.getLine()+1, std::pair<Character,int>(character,0x4));

        } else {
            qDebug().noquote() << "There is nothing after the char \n inserted";
            style.emplace_back(std::pair<Character,int>(character,0x4));
        }
    }

    structure[pos.getLine()].insert(structure[pos.getLine()].begin() + pos.getCh(), character); // insert the character in the pos.
}

void CRDT::insertIntoTextDocument(char character, QTextCharFormat charFormat, Pos pos){

    int oldCursorPos = textCursor.position();

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
    textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());

    textCursor.insertText(QString{character});
    textCursor.setPosition(textCursor.position()-1, QTextCursor::KeepAnchor);
    textCursor.mergeCharFormat(charFormat);

    textCursor.setPosition(oldCursorPos);
}


// handle style changed

Pos CRDT::handleStyleChanged(const Character &character) {
    Pos pos = this->findPosition(character);

    this->structure[pos.getLine()][pos.getCh()].setTextCharFormat(character.getTextCharFormat());

    this->changeStyleOfDocument(pos,character.getTextCharFormat());

    return pos;
}

void CRDT::changeStyleOfDocument(Pos pos, const QTextCharFormat &textCharFormat) {
    int oldCursorPos = textCursor.position();

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
    textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());


    textCursor.setPosition(textCursor.position() + 1, QTextCursor::KeepAnchor);
    textCursor.mergeCharFormat(textCharFormat);

    textCursor.setPosition(oldCursorPos);
}


// handle alignment

void CRDT::handleAlignmentChanged(int alignment, int blockNumber){

    int oldCursorPos = this->textCursor.position();

    QTextBlockFormat f=this->textCursor.blockFormat();
    this->textCursor.movePosition(QTextCursor::Start);

    this->textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, blockNumber);

    int cursorPos = this->textCursor.position();

    Qt::Alignment a(alignment);

    qDebug() << a;

    f.setAlignment(a);

    this->textCursor.setBlockFormat(f);


    if(blockNumber<this->style.size()) {
        this->style[blockNumber].second=alignment;
    }


    this->textCursor.setPosition(oldCursorPos);


}


// handle delete

void CRDT::handleDelete(const Character &character) {
	Pos pos = this->findPosition(character);

	if (pos) {
		this->structure[pos.getLine()].erase(this->structure[pos.getLine()].begin() + pos.getCh());

		if (character.getValue() == '\n') {
            qDebug() << "Deleting: " << character.getValue() << "in pos: " << pos.getLine() << pos.getCh();

		    this->mergeLines(pos.getLine());

            if(style.size() > pos.getLine() + 1) {
                this->removeStyleLine(pos.getLine()+1);
            }
		}
		
		this->removeEmptyLines();
	}

	this->deleteFromTextDocument(pos);

    // print the structure for debugging
    qDebug() << "server/CRDT.cpp - handleDelete()     ---------- STRUCTURE ----------";
    for (int i = 0; i < structure.size(); i++) {
        for (int j = 0; j < structure[i].size(); j++) {
            QDebug qD(QtDebugMsg);
            char val = structure[i][j].getValue();
            int counter = structure[i][j].getCounter();
            QString siteId = structure[i][j].getSiteId();
            QString value = ""; if(val == '\n') value += "\n"; else value += val;
            qD << "                                     val:" << value << "  siteId: " << siteId << "  counter:" << counter << "  position:";
            std::vector<Identifier> identifier = structure[i][j].getPosition();
            for (Identifier id : identifier) {
                qD << id.getDigit();
            }
        }
    }
    qDebug() << ""; // newLine
}

void CRDT::deleteFromTextDocument(Pos pos){
    int oldCursorPos = textCursor.position();

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
    textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());

    textCursor.deleteChar();

    textCursor.setPosition(oldCursorPos);
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

void CRDT::removeEmptyLines() {
    for (int line = 0; line < this->structure.size(); line++) {
        if (this->structure[line].size() == 0) {
            this->structure.erase(this->structure.begin() + line);
            line--;
        }
    }
}

void CRDT::mergeLines(int line) {
    if(structure.size() > line + 1 && structure[line + 1].size() > 0) {
        structure[line].insert(structure[line].end(), structure[line + 1].begin(), structure[line + 1].end());
        structure.erase(structure.begin() + line + 1);
    }
}



/**
 * Serialization function for WRITING CRDT object to json
 * @param json
 */
void CRDT::write(QJsonObject &json) const {

	//TODO WRITE VECTOR OF CHARACTERS
	QJsonArray vectorVectors;
	for (const std::vector<Character> &vectorChar : structure) {
		QJsonArray arrayChar;
		for (const Character &character : vectorChar) {
			QJsonObject charObject;
			character.write(charObject);
			arrayChar.append(charObject);
		}
		vectorVectors.append(arrayChar);
	}
	json["structure"] = vectorVectors;
}

/**
 * Serialization function for READING CRDT object from json
 * @param json
 */
void CRDT::read(const QJsonObject &json) {

	//TODO READ VECTOR OF CHARACTERS
	if (json.contains("structure") && json["structure"].isArray()) {
		QJsonArray vectorVectors = json["structure"].toArray();
		structure.clear();
		structure.reserve(vectorVectors.size());
		for (int vectorIndex = 0; vectorIndex < vectorVectors.size(); vectorIndex++) {
			QJsonArray vectorChars = vectorVectors[vectorIndex].toArray();
			structure[vectorIndex].clear();
			structure[vectorIndex].reserve(vectorChars.size());
			std::vector<Character> innerVector;
			for (int indexChar = 0; indexChar < vectorChars.size(); ++indexChar) {
				QJsonObject charObject = vectorChars[indexChar].toObject();
				Character character;
				character.read(charObject);
				innerVector.push_back(character);
			}
			structure.push_back(innerVector);
		}
	}
}

/**
 * Function for saving CRDT to json file TODO change save to binary if all works
 * @return
 */
bool CRDT::saveCRDT(QString filename) {
	QFile saveFile(filename + ".json");

	if (!saveFile.open(QIODevice::WriteOnly)) {
		qWarning("Couldn't open save file.");
		return false;
	}

	QJsonObject CRDTobject;
	write(CRDTobject);
	QJsonDocument saveDocument(CRDTobject);
	saveFile.write(saveDocument.toJson());
	return true;
}

/**
 * Function for loading CRDT from json file
 * @return
 */
bool CRDT::loadCRDT(QString filename) {
	QFile loadFile(filename + ".json");

	if (!loadFile.open(QIODevice::ReadOnly)) {
		qWarning("Couldn't open save file.");
		return false;
	}

	QByteArray savedData = loadFile.readAll();
	QJsonDocument loadDocument(QJsonDocument::fromJson(savedData));
	read(loadDocument.object());
	return true;
}

int CRDT::getRow(Character blockId) {
    if (this->style.empty() || blockId.compareTo(this->style[0].first) < 0) {
        return -1;
    }

    int minLine = 0;
    int totalLines = this->style.size();
    int maxLine = totalLines - 1;

    Character lastBlockId = style[maxLine].first;

    // char is greater than all existing chars (insert at end)
    if (blockId.compareTo(lastBlockId) > 0) {
        return -1;
    }

    // binary search
    while (minLine <= maxLine) {
        int midLine = std::floor((minLine + maxLine) / 2);

        lastBlockId = style[midLine].first;

        if (blockId.compareTo(lastBlockId) == 0) {
            return midLine;
        } else if (blockId.compareTo(lastBlockId) < 0) {
            maxLine = midLine-1;
        } else {
            minLine = midLine+1;
        }
    }

    return -1;
}


void CRDT::removeStyleLine(int i) {
    this->style.erase(this->style.begin()+1);
}

std::vector<std::pair<Character,int>> CRDT::getStyle() {
    return this->style;
}