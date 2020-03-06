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
public:
//	enum class Permission {
//		READ, WRITE
//	};

private:
	QSqlDatabase db;

	QString connectionName;

	static QString generateSalt();

	static QString hashUsername(QString username);

	static QString hashPassword(QString password, QString salt);

	void initTables();

public:
	Database();

	Database(QString threadID);

	bool registerUser(QString username, QString password);

	bool authenticateUser(QString username, QString password);

	bool changeAvatar(QString username, const QByteArray &image);

	QByteArray getAvatar(QString username);

	bool createFile(QString fileID, QString userOwner);

	bool isOwner(QString fileID, QString username);

	bool addPermission(QString fileID, QString owner, QString username);

	std::map<QString, bool> getFiles(QString username);

	bool changeUsername(QString oldUsername, QString newUsername);

	bool changePassword(QString username, QString newPassword);

	QStringList getUsers(QString filename);

	bool changeFileName(QString oldFilename, QString newFilename);

	bool removePermission(QString filename, QString username);

	bool deleteFile(QString filename);

	bool createThreadConnection(QString threadID);

	bool testConnection();
};


#endif //TEXTEDITOR_DATABASE_H
