//
// Created by andrea settimo on 2019-09-02.
//

#include "User.h"

User::User(QString username): username(username){}

void User::setFileLis(QStringList fileList) {
    this->fileList = fileList;
}

QStringList User::getFileList() const{
    return fileList;
}

void User::setIsLogged(bool isLogged) {
    User::isLogged = isLogged;
}

bool User::isIsLogged() const {
    return isLogged;
}
