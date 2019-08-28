//
// Created by simone on 07/08/19.
//

#include "Version.h"

Version::Version(QString siteId) : siteId(siteId), counter() {}

int Version::getCounter() const {
	return counter;
}

const QString &Version::getSiteId() const {
	return siteId;
}

void Version::increment() {
	this->counter++;
}

void Version::read(const QJsonObject &json) {
	if (json.contains("counter") && json["counter"].isDouble())
		counter = json["counter"].toInt();
	if (json.contains("siteId") && json["siteId"].isString())
		siteId = json["counter"].toString();
}

void Version::write(QJsonObject &json) const {
	json["counter"] = counter;
	json["siteId"] = siteId;
}
