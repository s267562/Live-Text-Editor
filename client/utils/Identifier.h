//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_IDENTIFIER_H
#define TEXTEDITOR_IDENTIFIER_H

#include <string>
#include <QtCore/QString>
#include <QtCore/QJsonObject>

class Identifier {
public:
    Identifier(int digit, const QString &siteId);
    Identifier();

    int getDigit() const;
    const QString &getSiteId() const;
	void read(const QJsonObject &json);
	void write( QJsonObject &json) const;
    int compareTo(Identifier otherIdentifier);

    friend bool operator==(const Identifier &lhs, const Identifier &rhs);

    friend bool operator!=(const Identifier &lhs, const Identifier &rhs);

private:
    int digit;
    QString siteId;
};


#endif //TEXTEDITOR_IDENTIFIER_H
