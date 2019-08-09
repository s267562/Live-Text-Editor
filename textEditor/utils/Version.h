//
// Created by simone on 07/08/19.
//

#ifndef TEXTEDITOR_VERSION_H
#define TEXTEDITOR_VERSION_H


#include <string>

class Version {
public:
    Version(std::string siteId);

    int getCounter() const;
    const std::string &getSiteId() const;
    void increment();

private:
    int counter;
    std::string siteId;
};


#endif //TEXTEDITOR_VERSION_H
