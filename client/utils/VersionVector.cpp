//
// Created by simone on 07/08/19.
//

#include <QtCore/QString>
#include <QtCore/QJsonArray>
#include "VersionVector.h"

VersionVector::VersionVector(QString siteId) : localVersion(siteId) {
	this->versions = std::vector<Version>{};
	this->versions.push_back(this->localVersion);
}

const Version &VersionVector::getLocalVersion() const {
	return localVersion;
}

const std::vector<Version> &VersionVector::getVersions() const {
	return versions;
}

void VersionVector::increment() {
	this->localVersion.increment();
}

/*
 * updates vector with new version received from another site if vector doesn't contain version,
 * it's created and added to vector create exceptions if need be.
 */
void VersionVector::update(Version incomingVersion) {
	// TODO ?
}

void VersionVector::write(QJsonObject &json) const {
	QJsonObject versionObject;
	localVersion.write(versionObject);
	json["localVersion"] = versionObject;

	QJsonArray versionArray;
	for( const Version &version : versions){
		QJsonObject versionObj;
		version.write(versionObj);
		versionArray.append(versionObj);
	}
	json["versions"] = versionArray;
}

void VersionVector::read(const QJsonObject &json) {
	if(json.contains("localVersion") && json["localVersion"].isObject())
		localVersion.read(json["localVersion"].toObject());

	if(json.contains("versions") && json["versions"].isArray()){
		QJsonArray versionArray = json["versions"].toArray();
		versions.clear();
		versions.reserve(versionArray.size());
		for (int index = 0 ; index< versionArray.size(); index++){
			QJsonObject versionObj = versionArray[index].toObject();
			Version ver(nullptr);
			ver.read(versionObj);
			versions.push_back(ver);
		}
	}
}



// TODO metodi hasBeenApplied, getVersionFromVector ?