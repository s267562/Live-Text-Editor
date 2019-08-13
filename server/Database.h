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

	static QString generateSalt();

	static QString hashUsername(QString username);

	static QString hashPassword(QString password, QString salt);

public:
	Database();

	bool registerUser(QString username,QString password);

	bool authenticateUser(QString username, QString password);

	bool changeAvatar(QString username, const QByteArray &image);

	QByteArray getAvatar(QString username);
};


#endif //TEXTEDITOR_DATABASE_H
