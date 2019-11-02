//
// Created by andrea settimo on 2019-09-02.
//

#ifndef TEXTEDITOR_USER_H
#define TEXTEDITOR_USER_H

#include <QString>
#include <QStringList>
#include <map>
#include <QPixmap>

class User {
    QString username;
    std::map<QString, bool> fileList;
    bool isLogged;
    QPixmap avatar;

public:
    User(QString username, QPixmap avatar);
    void setUsername(const QString &username);
    void setFileList(std::map<QString, bool> fileList);
    void setIsLogged(bool isLogged);
    bool isIsLogged() const;
    const QString &getUsername() const;
    const QPixmap &getAvatar() const;
    void setAvatar(const QPixmap &avatar);
    std::map<QString, bool> getFileList() const;
    void addFile(QString file);
};


#endif //TEXTEDITOR_USER_H
