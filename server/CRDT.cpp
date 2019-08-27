//
// Created by simone on 05/08/19.
//

#include <ctime>
#include <cmath>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include "CRDT.h"

CRDT::CRDT() {
	this->structure = {{}}; // TODO inizializzare il vettore structure?
}


Character CRDT::handleInsert(char val, Pos pos, QString siteId) {
	this->vector.increment();
	const Character character = generateChar(val, pos);
	insertChar(character, pos);
	std::cout << val << " inserted." << std::endl;

	// print the structure for debugging
	std::cout << "-----------------------" << std::endl << "STRUCTURE:" << std::endl;
	for (int i = 0; i < structure.size(); i++) {
		for (int j = 0; j < structure[i].size(); j++) {
			char val = structure[i][j].getValue();
			int counter = structure[i][j].getCounter();
			std::cout << "val = " << val << "; counter = " << counter << "; position: ";
			std::vector<Identifier> identifier = structure[i][j].getPosition();
			for (Identifier id : identifier) {
				std::cout << id.getDigit() << " ";
			}
			std::cout << std::endl;
		}
	}
	return character;
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

std::vector<Identifier>
CRDT::generatePosBetween(std::vector<Identifier> pos1, std::vector<Identifier> pos2, std::vector<Identifier> newPos,
						 int level) {

	// change 2 to any other number to change base multiplication
	int base = pow(2, level) * CRDT::base;
	// TODO? char boundaryStrategy = this.retrieveStrategy(level);

	Identifier id1{0, this->siteId}, id2{base, this->siteId};
	if (pos1.size() > 0) id1 = pos1[0];
	if (pos2.size() > 0) id2 = pos2[0];

	if (id2.getDigit() - id1.getDigit() > 1) {
		int newDigit = this->generateIdBetween(id1.getDigit(), id2.getDigit());
		Identifier newId{newDigit, this->siteId};
		newPos.push_back(newId);
		return newPos;
	} else if (id2.getDigit() - id1.getDigit() == 1) {
		newPos.push_back(id1);

		std::vector<Identifier> pos1_2;
		if (pos1.size() > 0) pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
		return this->generatePosBetween(pos1_2, std::vector<Identifier>{}, newPos, level + 1);

	} else /* if (id1.getDigit() == id2.getDigit()) */ {
		if (id1.getSiteId() < id2.getSiteId()) {
			newPos.push_back(id1);
			std::vector<Identifier> pos1_2;
			if (pos1.size() > 0) pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
			return this->generatePosBetween(pos1_2, std::vector<Identifier>{}, newPos, level + 1);
			return this->generatePosBetween(std::vector<Identifier>(pos1.begin() + 1, pos1.end()),
											std::vector<Identifier>{}, newPos, level + 1);
		} else /* if (id1.getSiteId() == id2.getSiteId()) */ {
			newPos.push_back(id1);
			std::vector<Identifier> pos1_2, pos2_2;
			if (pos1.size() > 0) pos1_2 = std::vector<Identifier>(pos1.begin() + 1, pos1.end());
			if (pos2.size() > 0) pos2_2 = std::vector<Identifier>(pos2.begin() + 1, pos2.end());
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
	srand(time(0)); // seed for different random value
	return (int) floor(((double) rand() / (RAND_MAX)) * (max - min)) +
		   min; // ((double) rand() / (RAND_MAX)) -> random value [0, 1)
}

void CRDT::insertChar(Character character, Pos pos) {

	if (pos.getLine() == structure.size()) {
		structure.push_back(std::vector<Character>{}); // pushing a new line.
	}

	// if inserting a newline, split line into two lines.
	if (character.getValue() == '\n') {
		std::vector<Character> lineAfter(structure[pos.getLine()].begin() + pos.getCh(),
										 structure[pos.getLine()].end()); // get line after.
		if (lineAfter.size() != 0) {
			structure.insert(structure.begin() + pos.getLine() + 1, lineAfter); // insert line after.
		}
	}
	structure[pos.getLine()].insert(structure[pos.getLine()].begin() + pos.getCh(),
									character); // insert the character in the pos.
}


void CRDT::handleDelete(const Character &character) {
	Pos pos = this->findPosition(character);

	if (pos) {
		this->structure[pos.getLine()].erase(this->structure[pos.getLine()].begin() + pos.getCh());

		if (character.getValue() == '\n' && this->structure.size() > pos.getLine()) {
			this->mergeLines(pos.getLine());
		}

		this->removeEmptyLines();
	}

}

/**
 * Serialization function for WRITING CRDT object to json
 * @param json
 */
void CRDT::write(QJsonObject &json) const {
	json["siteId"] = siteId;

	QJsonObject versionVectorObj;
	vector.write(versionVectorObj);
	json["vector"] = versionVectorObj;

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
	if (json.contains("siteId") && json["siteId"].isString())
		siteId = json["siteId"].toString();

	if (json.contains("vector") && json["vector"].isObject()) {
		vector.read(json["vector"].toObject());
	}

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

Pos CRDT::findPosition(Character character) {
	// check if struct is empty or char is less than first char
	if (this->structure.empty() || character.compareTo(this->structure[0][0]) < 0) {
		return Pos{-1, -1}; // false obj
	}

	int minLine = 0;
	int totalLines = this->structure.size();
	int maxLine = totalLines - 1;
	std::vector<Character> lastLine = this->structure[maxLine];

	Character lastChar = lastLine[lastLine.size() - 1];

	// char is greater than all existing chars (insert at end)
	if (character.compareTo(lastChar) > 0) {
		return Pos{-1, -1}; // false obj
	}

	// binary search
	while (minLine + 1 < maxLine) {
		int midLine = std::floor(minLine + (maxLine - minLine) / 2);
		std::vector<Character> currentLine = this->structure[midLine];
		lastChar = currentLine[currentLine.size() - 1];

		if (character.compareTo(lastChar) == 0) {
			return Pos{midLine, (int) currentLine.size() - 1};
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
		return {minLine, charIdx};
	} else {
		int charIdx = this->findIndexInLine(character, maxCurrentLine);
		return {maxLine, charIdx};
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
	if (structure.size() > line + 1 && structure[line + 1].size() > 0) {
		structure[line].insert(structure[line].end(), structure[line + 1].begin(), structure[line + 1].end());
	}
}

