//
// Created by andrea settimo on 2019-09-02.
//

#include "User.h"

#include <utility>

User::User(QString username, const QPixmap& avatar): username(std::move(username)), avatar(avatar){}

void User::setFileList(std::map<QString, bool> fileList) {
    this->fileList = fileList;
}

std::map<QString, bool> User::getFileList() const{
    return fileList;
}

void User::setIsLogged(bool isLogged) {
    User::isLogged = isLogged;
}

bool User::isIsLogged() const {
    return isLogged;
}

const QString &User::getUsername() const {
    return username;
}

void User::setUsername(const QString &username) {
    User::username = username;
}

const QPixmap &User::getAvatar() const {
    return avatar;
}

void User::setAvatar(const QPixmap &avatar) {
    User::avatar = avatar;
}

void User::addFile(const QString& file) {
    fileList[file] = true;
}
