//
// Created by simone on 06/08/19.
//

#include "Identifier.h"

Identifier::Identifier(int digit, const QString &siteId) : digit(digit), siteId(siteId) {}

int Identifier::getDigit() const {
	return digit;
}

const QString &Identifier::getSiteId() const {
	return siteId;
}

int Identifier::compareTo(Identifier otherIdentifier) {
	if (this->getDigit() < otherIdentifier.getDigit()) {
		return -1;
	} else if (this->getDigit() > otherIdentifier.getDigit()) {
		return 1;
	} else {
	    if(this->getSiteId() < otherIdentifier.getSiteId()) {
            return -1;
	    } else if(this->getSiteId() > otherIdentifier.getSiteId()) {
            return 1;
	    } else {
	        return 0;
	    }
	}
}

/**
 * Serialization function for WRITING Identifier object to json
 * @param json
 */
void Identifier::write(QJsonObject &json) const {
	json["siteId"] = siteId;
	json["digit"] = digit;
}

/**
* Serialization function for READING Identifier object to json
* @param json
*/
void Identifier::read(const QJsonObject &json) {
	if (json.contains("digit") && json["digit"].isDouble())
		digit = json["digit"].toInt();
	if (json.contains("siteId") && json["siteId"].isString())
		siteId = json["siteId"].toString();
}

Identifier::Identifier() {
	digit = -1;
	siteId = nullptr;
}

bool operator==(const Identifier &lhs, const Identifier &rhs) {
    return lhs.digit == rhs.digit &&
           lhs.siteId == rhs.siteId;
}

bool operator!=(const Identifier &lhs, const Identifier &rhs) {
    return !(rhs == lhs);
}

void Identifier::setSiteId(const QString &siteId) {
    Identifier::siteId = siteId;
}
