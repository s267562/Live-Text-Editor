//
// Created by simone on 07/08/19.
//

#ifndef TEXTEDITOR_VERSION_H
#define TEXTEDITOR_VERSION_H


#include <string>
#include <QtCore/QString>

class Version {
public:
    Version(QString siteId);

    int getCounter() const;
    const QString &getSiteId() const;
    void increment();

private:
    int counter;
    QString siteId;
};


#endif //TEXTEDITOR_VERSION_H
