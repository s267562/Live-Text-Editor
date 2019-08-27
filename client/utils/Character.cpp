//
// Created by simone on 06/08/19.
//

#include <QtCore/QJsonArray>
#include "Character.h"

Character::Character(char value, int counter, const QString &siteId, const std::vector<Identifier> &position)
		: value(value), counter(counter), siteId(siteId), position(position) {}

int Character::compareTo(Character otherCharacter) {
	std::vector<Identifier> pos1 = this->getPosition();
	std::vector<Identifier> pos2 = otherCharacter.getPosition();

	for (int i = 0; i < std::min(pos1.size(), pos2.size()); i++) {
		Identifier id1 = pos1[i];
		Identifier id2 = pos2[i];
		int comp = id1.compareTo(id2);

		if (comp != 0) {
			return comp;
		}
	}

	if (pos1.size() < pos2.size()) {
		return -1;
	} else if (pos1.size() > pos2.size()) {
		return 1;
	} else {
		return 0;
	}
}

char Character::getValue() const {
	return value;
}

int Character::getCounter() const {
	return counter;
}

const QString &Character::getSiteId() const {
	return siteId;
}

const std::vector<Identifier> &Character::getPosition() const {
	return position;
}

/**
* Serialization function for READING Character object to json
* @param json
*/
void Character::read(const QJsonObject &json) {
	if (json.contains("value") && json["value"].isDouble())
		value = static_cast<char>(json["value"].toInt());
	if (json.contains("counter") && json["counter"].isDouble())
		counter = json["counter"].toInt();
	if (json.contains("siteId") && json["siteId"].isString())
		siteId = json["siteId"].toString();

	if (json.contains("position") && json["position"].isArray()) {
		QJsonArray positionArray = json["position"].toArray();
		position.clear();
		position.reserve(positionArray.size());
		for (int i = 0; i < positionArray.size(); ++i) {
			QJsonObject identifierObj = positionArray[i].toObject();
			Identifier ident;
			ident.read(identifierObj);
			position.push_back(ident);
		}
	}
}

/**
* Serialization function for WRITING Identifier object to json
* @param json
*/
void Character::write(QJsonObject &json) const {
	json["value"] = value;
	json["counter"] = counter;
	json["siteId"] = siteId;

	QJsonArray identifierArray;
	for (const Identifier &identifier : position) {
		QJsonObject identifierObject;
		identifier.write(identifierObject);
		identifierArray.append(identifierObject);
	}
	json["position"] = identifierArray;
}

Character::Character() {
	value = '*';
	counter = -1;
	siteId = nullptr;
	position = std::vector<Identifier>();
}
