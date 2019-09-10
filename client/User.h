//
// Created by andrea settimo on 2019-09-02.
//

#ifndef TEXTEDITOR_USER_H
#define TEXTEDITOR_USER_H

#include <QString>
#include <QStringList>
#include <map>

class User {
    QString username;
public:
    void setUsername(const QString &username);

private:
    std::map<QString, bool> fileList;
    bool isLogged;

public:
    User(QString username);
    void setFileLis(std::map<QString, bool> fileList);
    std::map<QString, bool> getFileList() const;
    void setIsLogged(bool isLogged);
    bool isIsLogged() const;
    const QString &getUsername() const;

};


#endif //TEXTEDITOR_USER_H
