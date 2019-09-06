//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_CHARACTER_H
#define TEXTEDITOR_CHARACTER_H

#include <string>
#include <vector>
#include <QtCore/QString>
#include "Identifier.h"
#include "CharFormat.h"
#include <QJsonDocument>

class Character {
public:
    Character(char value, CharFormat charFormat, int counter, const QString &siteId, const std::vector<Identifier> &position);
	Character();
    char getValue() const;
    const CharFormat &getCharFormat() const;
    void setCharFormat(const CharFormat &charFormat);
    int getCounter() const;
    const QString &getSiteId() const;
    const std::vector<Identifier> &getPosition() const;
	void read(const QJsonObject &json);
	void write( QJsonObject &json) const;
    int compareTo(Character otherCharacter);
    QByteArray toQByteArrayInsertVersion();
    static Character toCharacterInsertVersion(QJsonDocument jsonDocument);
	QByteArray toQByteArrayDeleteVersion();
	static Character toCharacterDeleteVersion(QJsonDocument jsonDocument);

private:
    char value;
    CharFormat charFormat;
    int counter;
    QString siteId;
    std::vector<Identifier> position;
};


#endif //TEXTEDITOR_CHARACTER_H
