/**
 * Created by Davide Sordi
 * Using CLion
 * Date: 12/08/2019
 * Time: 10.50
 * 
 * Class: Database
 * Project: textEditor
 */

#ifndef TEXTEDITOR_DATABASE_H
#define TEXTEDITOR_DATABASE_H

#include <QtSql>
#include <QCryptographicHash>
#include <string>

class Database {
private:
	QSqlDatabase db;

	static std::string generateSalt();

	static std::string hashUsername(std::string username);

	static std::string hashPassword(std::string password, std::string salt);

public:
	Database();

	bool registerUser(std::string username, std::string password);

	bool authenticateUser(std::string username, std::string password);

	bool changeAvatar(std::string username, const QByteArray &image);

	QByteArray getAvatar(std::string username);
};


#endif //TEXTEDITOR_DATABASE_H
