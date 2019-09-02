//
// Created by andrea settimo on 2019-09-02.
//

#ifndef TEXTEDITOR_USER_H
#define TEXTEDITOR_USER_H

#include <QString>
#include <QStringList>

class User {
    QString username;
    QStringList fileList;
    bool isLogged;

public:
    User(QString username);
    void setFileLis(QStringList fileList);
    QStringList getFileList() const;
    void setIsLogged(bool isLogged);
    bool isIsLogged() const;

};


#endif //TEXTEDITOR_USER_H
