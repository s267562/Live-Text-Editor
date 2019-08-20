//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_IDENTIFIER_H
#define TEXTEDITOR_IDENTIFIER_H

#include <string>
#include <QtCore/QString>

class Identifier {
public:
    Identifier(int digit, const QString &siteId);

    int getDigit() const;
    const QString &getSiteId() const;

    int compareTo(Identifier otherIdentifier);

private:
    int digit;
    QString siteId;
};


#endif //TEXTEDITOR_IDENTIFIER_H
