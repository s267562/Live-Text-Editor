//
// Created by simone on 06/08/19.
//

#ifndef TEXTEDITOR_IDENTIFIER_H
#define TEXTEDITOR_IDENTIFIER_H

#include <string>

class Identifier {
public:
    Identifier(int digit, const std::string &siteId);

    int getDigit() const;
    const std::string &getSiteId() const;

private:
    int digit;
    std::string siteId;
};


#endif //TEXTEDITOR_IDENTIFIER_H
