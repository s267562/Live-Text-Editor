//
// Created by andrea settimo on 2019-09-02.
//

#ifndef TEXTEDITOR_USER_H
#define TEXTEDITOR_USER_H

#include <QString>
#include <QStringList>
#include <map>
#include <QPixmap>
#include <QMap>

class User {
    QString username;
    QMap<QString, bool> fileList;
    bool isLogged{};
    QPixmap avatar;

public:
    User(QString username, const QPixmap& avatar);
    void setUsername(const QString &username);
    void setFileList(QMap<QString, bool> fileList);
    void setIsLogged(bool isLogged);
    bool isIsLogged() const;
    const QString &getUsername() const;
    const QPixmap &getAvatar() const;
    void setAvatar(const QPixmap &avatar);
    QMap<QString, bool> getFileList() const;
    void addFile(const QString& file);
};


#endif //TEXTEDITOR_USER_H
