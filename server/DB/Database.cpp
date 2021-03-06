/**
 * Created by Davide Sordi
 * Using CLion
 * Date: 12/08/2019
 * Time: 10.50
 * 
 * Class: Database
 * Project: textEditor
 */

#include <iostream>
#include <QtSql/QSqlQuery>
#include <utility>
#include "Database.h"

/**
 * Constructor, initialize a new DB if necessary
 */
Database::Database() {
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName("database.sqlite");

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return;
	}

	// Check DB was created empty
	QString checkEmptyQuery = "SELECT * FROM sqlite_master WHERE name = 'users' OR name = 'files'";

	// CREATE TABLE query string
	QString createTableQuery =
			"CREATE TABLE users ("
			"username VARCHAR(256) PRIMARY KEY,"
			"password VARCHAR(256),"
			"salt VARCHAR(128),"
			"avatar BLOB);";

	QString createTableQueryFiles =
			"CREATE TABLE files ("
			"fileID VARCHAR(256),"
			"username VARCHAR(256),"
			"owner INT,"
			"read INT,"
			"write INT,"
			"PRIMARY KEY (fileID, username));";

	// Query executions
	QSqlQuery qry;
	if (qry.exec(checkEmptyQuery)) { // check db is empty
		if (qry.next())
			qDebug() << "DB is OK";
		else {
			qDebug() << "DB need table creation";
			// Query execution
			if (!qry.exec(createTableQuery))
				qDebug() << "Error creating table users";
			if (!qry.exec(createTableQueryFiles))
				qDebug() << "Error creating table files\n" << qry.lastError();
		}
	}
	db.close();
}

/**
 * Function for generating salt hashed sha256
 * @return sha256 hash of salt
 */
QString Database::generateSalt() {
	auto timestamp = std::chrono::system_clock::now().time_since_epoch();
	QString salt = QString::fromStdString(std::to_string(timestamp.count()));
	auto hashedSalt = QCryptographicHash::hash(salt.toLocal8Bit(), QCryptographicHash::Sha256);
	return QString(hashedSalt.toHex());
}

/**
 * Function for hashing username, for preventing sql injection instead of sanitizing string
 * @param username : username to hash
 * @return : hashed username
 */
QString Database::hashUsername(QString username) {
	QString toHash = std::move(username);
	auto hashedUsername = QCryptographicHash::hash(QByteArray(toHash.toLocal8Bit()), QCryptographicHash::Sha256);
	return QString(hashedUsername.toHex());
}

/**
 * Function for hashing a password provided a salt string
 * @param password : password to hash
 * @param salt : salt to add to password before hash
 * @return : hashed password SHA256
 */
QString Database::hashPassword(QString password, QString salt) {
	QString toHash = std::move(password) + std::move(salt);
	auto hashedPassword = QCryptographicHash::hash(toHash.toLocal8Bit(), QCryptographicHash::Sha256);
	return QString(hashedPassword.toHex());
}

/**
 * Function for registering a new user.
 * @param username : username to use
 * @param password : password to use
 * @return : true if correctly registered, false if username already registered
 */
bool Database::registerUser(QString username, QString password) {
	bool result = false; // result of registration
	QString hashedUsername = hashUsername(std::move(username));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QString querySelect = "SELECT * FROM users WHERE username='" + hashedUsername + "'";
	QSqlQuery checkUsernameQuery;

	if (checkUsernameQuery.exec(querySelect)) {
		if (checkUsernameQuery.isActive()) {
			if (!checkUsernameQuery.next()) {
				// Username free for use, we can register the user

				QString salt = generateSalt();
				QString hashedPassword = hashPassword(std::move(password), salt);

				QSqlQuery qry;
				qry.prepare("INSERT INTO users ("
							"username,"
							"password,"
							"salt)"
							"VALUES (?,?,?);");

				qry.addBindValue(hashedUsername);
				qry.addBindValue(hashedPassword);
				qry.addBindValue(salt);

				if (!qry.exec())
					qDebug() << "Error inserting query";
				else result = true;
			}
		}
	}
	db.close();
	return result;
}

/**
 * Function for authenticating a registered user
 * @param username : username given by user
 * @param password : password given
 * @return : true if all correct
 */

bool Database::authenticateUser(QString username, QString password) {
	bool result = false;
	QString hashedUsername = hashUsername(std::move(username));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QSqlQuery authenticationQuery;
	authenticationQuery.prepare("SELECT username, password, salt FROM users WHERE username=:username");

	authenticationQuery.bindValue(":username", hashedUsername);

	if (authenticationQuery.exec()) {
		if (authenticationQuery.isActive()) {
			if (authenticationQuery.first()) {
				// Username is registered
				QString salt = authenticationQuery.value(2).toString();
				QString hashedPassword = hashPassword(std::move(password), salt);
				QString passwordFromDB = authenticationQuery.value(1).toString();

				if (passwordFromDB == hashedPassword)
					// OK, username and password are true
					result = true;
			}
		}
	}
	db.close();
	return result;
}

/**
 * Function for changing the user's avatar
 * @param username : username to change the avatar
 * @param image :QByteArray of the image to load
 * @return true if image changed, else false
 */
bool Database::changeAvatar(QString username, const QByteArray &image) {
	bool result = false;
	QString hashedUsername = hashUsername(std::move(username));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QSqlQuery query;
	query.prepare("UPDATE users SET avatar=:avatar WHERE username=:username");
	query.bindValue(":avatar", image);
	query.bindValue(":username", hashedUsername);

	if (!query.exec()) {
		qDebug() << "Error inserting image into table:\n" << query.lastError();
	} else
		result = true;

	db.close();
	return result;
}

/**
 * Get user's avatar
 * @param username : usrname of the user requested avatar
 * @return : QbyteArray of the avatar, or "NULL" string if user doesn't have an avatar yet. May be nullptr in case of error
 */
QByteArray Database::getAvatar(QString username) {
	QString hashedUsername = hashUsername(std::move(username));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return nullptr;
	}

	QSqlQuery query;
	query.prepare("SELECT avatar FROM users WHERE username=:username");
	query.bindValue(":username", hashedUsername);

	if (!query.exec())
		qDebug() << "Error getting image from table:\n" << query.lastError();
	query.first();
	QByteArray avatar = query.value(0).toByteArray();
	if(avatar.isNull())
		avatar.append("NULL");
	db.close();
	return avatar;
}

/**
 * Register a new file and his owner into the DB
 * @param fileID : file identifier
 * @param userOwner : username of the user owner of the file
 * @return : true if correctly registered, false if error
 */
bool Database::createFile(QString fileID, QString userOwner) {
	bool result = false;
	QString hashedUsername = hashUsername(std::move(userOwner));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QSqlQuery queryCheckUsername;
	queryCheckUsername.prepare("SELECT * FROM users WHERE username=:username");
	queryCheckUsername.bindValue(":username", hashedUsername);

	if (!queryCheckUsername.exec()){
		qDebug() << "Error getting username from table:\n" << queryCheckUsername.lastError();
		db.close();
		return false;
	}

	// Username is valid, register file id
	if (queryCheckUsername.first()) {
		QSqlQuery queryRegisterFile;
		queryRegisterFile.prepare("INSERT INTO files ("
								  "fileID,"
								  "username,"
								  "owner,"
								  "read,"
								  "write)"
								  "VALUES ( ?,?,?,?,?)");
		queryRegisterFile.addBindValue(fileID);
		queryRegisterFile.addBindValue(hashedUsername);
		queryRegisterFile.addBindValue(true);
		queryRegisterFile.addBindValue(true);
		queryRegisterFile.addBindValue(true);

		if (!queryRegisterFile.exec())
			qDebug() << "Error registering new file:\n" << queryRegisterFile.lastError();
		else
			result = true;
	}
	db.close();
	return result;
}

/**
 * Check if a user is the owner of a given file
 * @param fileID : file identifier
 * @param username : username to check
 * @return : true if user is owner, false else
 */
bool Database::isOwner(QString fileID, QString username) {
	bool result = false;
	QString hashedUsername = hashUsername(std::move(username));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QSqlQuery queryCheckOwner;
	queryCheckOwner.prepare("SELECT * FROM files WHERE fileID=:fileID AND username=:username AND owner=:owner");
	queryCheckOwner.bindValue(":fileID", fileID);
	queryCheckOwner.bindValue(":username", hashedUsername);
	queryCheckOwner.bindValue(":owner", true);

	if (!queryCheckOwner.exec()) {
		qDebug() << "Error getting file owner from table:\n" << queryCheckOwner.lastError();
		db.close();
		return false;
	}

	// User is the owner
	if (queryCheckOwner.first()) {
		result = true;
	}
	db.close();
	return result;
}

/**
 * Function for adding permission for a file to a user
 * @param fileID : file to modify
 * @param username : user whe need permissions
 * @param permission : ENUM type for adding permission READ or WRITE
 * @return true if permissions modified, false else
 */
bool Database::addPermission(QString fileID, QString username, Database::Permission permission) {
	bool result = false;
	QString hashedUsername = hashUsername(std::move(username));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QSqlQuery queryCheckUser;
	queryCheckUser.prepare("SELECT owner, read ,write FROM files WHERE fileID=:fileID AND username=:username");
	queryCheckUser.bindValue(":fileID", fileID);
	queryCheckUser.bindValue(":username", hashedUsername);

	if (!queryCheckUser.exec()){
		qDebug() << "Error getting file permission for the user from table:\n" << queryCheckUser.lastError();
		db.close();
		return false;
	}

	// User already have some permissions
	if (queryCheckUser.first()) {
		if (queryCheckUser.value(0) == true) {
			// user is already the owner of this file, no further action
			result = true;
		}
		if (permission == Permission::WRITE) {
			// we need to give him write permission and also read
			QSqlQuery grantPermissionW;
			grantPermissionW.prepare(
					"UPDATE files SET write=:write, read=:read WHERE username=:username AND fileID=:fileID");
			grantPermissionW.bindValue(":write", true);
			grantPermissionW.bindValue(":read", true);
			grantPermissionW.bindValue(":username", hashedUsername);
			grantPermissionW.bindValue(":fileID", fileID);

			if (!grantPermissionW.exec())
				qDebug() << "Error setting WR file permission for the user:\n" << grantPermissionW.lastError();
			else
				result = true;
		}
		if (permission == Permission::READ) {
			//we need to give him read permission
			QSqlQuery grantPermissionR;
			grantPermissionR.prepare("UPDATE files SET read=:read WHERE username=:username AND fileID=:fileID");
			grantPermissionR.bindValue(":read", true);
			grantPermissionR.bindValue(":username", hashedUsername);
			grantPermissionR.bindValue(":fileID", fileID);

			if (!grantPermissionR.exec())
				qDebug() << "Error setting R file permission for the user:\n" << grantPermissionR.lastError();
			else
				result = true;
		}
	} else { //user permissions need to be registered
		QSqlQuery grantPermissions;
		grantPermissions.prepare(
				"INSERT INTO files (fileID,username,owner,read,write) VALUES (:fileID, :username, :owner, :read ,:write)");
		grantPermissions.bindValue(":fileID", fileID);
		grantPermissions.bindValue(":username", hashedUsername);
		grantPermissions.bindValue(":owner", false);
		grantPermissions.bindValue(":write", permission == Permission::WRITE);
		grantPermissions.bindValue(":read", true);

		if (!grantPermissions.exec())
			qDebug() << "Error setting new file permission for the user:\n" << grantPermissions.lastError();
		else
			result = true;
	}
	db.close();
	return result;
}

/**
 * Retrieve permissions for a given user and a given filename
 * @param fileID : filename
 * @param username : user
 * @return : List of permissions: --> [R,W] / [R] / [W] / empty  ------- [R,R,R] is returned in case of error
 */
QList<Database::Permission> Database::getPermissions(QString fileID, QString username) {
	QString hashedUsername = hashUsername(username);
	QList<Database::Permission> permissions;

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		permissions.append(Database::Permission::READ);
		permissions.append(Database::Permission::READ);
		permissions.append(Database::Permission::READ);
		return permissions;
	}

	QSqlQuery getPermissions;
	getPermissions.prepare("SELECT owner, read, write FROM files WHERE fileID=:fileID AND username=:username");
	getPermissions.bindValue(":fileID", fileID);
	getPermissions.bindValue(":username", hashedUsername);

	if (!getPermissions.exec()){
		qDebug() << "Error getting file permission for the user: " << username << "\n" << getPermissions.lastError();
		db.close();
		permissions.append(Database::Permission::READ);
		permissions.append(Database::Permission::READ);
		permissions.append(Database::Permission::READ);
		return permissions;
	}

	if (getPermissions.first()) {
		if (getPermissions.value(0) == true) {
			permissions.append(Database::Permission::READ);
			permissions.append(Database::Permission::WRITE);
		} else {
			if (getPermissions.value(1) == true)
				permissions.append(Database::Permission::READ);
			if (getPermissions.value(2) == true)
				permissions.append(Database::Permission::WRITE);
		}
	}
	db.close();
	return permissions;
}

///**
// * Retrieve all filenames owned by the given user
// * @param username : username to get files owned
// * @return : list of filenames (QString) --------- return list with single object "DBERROR" in case of error
// */
//QList<QString> Database::getFiles(QString username) {
//	QString hashedUsername = hashUsername(username);
//	QList<QString> userFiles;
//
//	// DB opening
//	if (!db.open()) {
//		qDebug() << "Error opening DB";
//		userFiles.append("DBERROR");
//		return userFiles;
//	}
//
//	QSqlQuery getUserFiles;
//	getUserFiles.prepare("SELECT fileID FROM files WHERE username=:username AND owner=:owner");
//	getUserFiles.bindValue(":username", hashedUsername);
//	getUserFiles.bindValue(":owner", true);
//
//	if (!getUserFiles.exec()) {
//		qDebug() << "Error getting files for the user: " << username << "\n" << getUserFiles.lastError();
//		db.close();
//		userFiles.append("DBERROR");
//		return userFiles;
//	}
//
//	while(getUserFiles.next()){
//		userFiles.push_back(getUserFiles.value(0).toString());
//	}
//	return userFiles;
//}

/**
 * Retrieve all filenames owned by the given user
 * @param username : username to get files owned
 * @return : list of filenames (QString,bool) --------- return list with single object "DBERROR,false" in case of error
 *  boolean value indicates the ownership for a file
 */
QList<std::pair<QString, bool>> Database::getFiles(QString username) {
	QString hashedUsername = hashUsername(username);
	QList<std::pair<QString, bool>> userFiles;

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		userFiles.append(std::make_pair("DBERROR", false));
		return userFiles;
	}

	QSqlQuery getUserOwnedFiles;
	getUserOwnedFiles.prepare("SELECT fileID FROM files WHERE username=:username AND owner=:owner");
	getUserOwnedFiles.bindValue(":username", hashedUsername);
	getUserOwnedFiles.bindValue(":owner", true);

	if (!getUserOwnedFiles.exec()) {
		qDebug() << "Error getting files for the user: " << username << "\n" << getUserOwnedFiles.lastError();
		db.close();
		userFiles.append(std::make_pair("DBERROR", false));
		return userFiles;
	}

	while(getUserOwnedFiles.next()){
		userFiles.push_back(std::make_pair(getUserOwnedFiles.value(0).toString(),true));
	}

	QSqlQuery getUserWriteFiles;
	getUserWriteFiles.prepare("SELECT fileID FROM files WHERE username=:username AND owner=:owner AND write=:write");
	getUserWriteFiles.bindValue(":username", hashedUsername);
	getUserWriteFiles.bindValue(":owner", false);
	getUserWriteFiles.bindValue(":write", true);

	if (!getUserWriteFiles.exec()) {
		qDebug() << "Error getting files for the user: " << username << "\n" << getUserWriteFiles.lastError();
		db.close();
		userFiles.clear();
		userFiles.append(std::make_pair("DBERROR", false));
		return userFiles;
	}

	while(getUserWriteFiles.next()){
		userFiles.push_back(std::make_pair(getUserWriteFiles.value(0).toString(),false));
	}

	return userFiles;
}

/**
 * Change the username of a given user
 * @param oldUsername : username to be changed
 * @param newUsername : new username to insert in DB
 * @return true in case of success.
 */
bool Database::changeUsername(QString oldUsername, QString newUsername) {
	bool result = false;
	QString hashedOldUsername = hashUsername(std::move(oldUsername));
	QString hashedNewUsername = hashUsername(std::move(newUsername));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QSqlQuery query;
	query.prepare("UPDATE users SET username=:newUsername WHERE username=:oldUsername");
	query.bindValue(":oldUsername", hashedOldUsername);
	query.bindValue(":newUsername", hashedNewUsername);

	if (!query.exec()) {
		qDebug() << "Error change username:\n" << query.lastError();
	} else
		result = true;

	db.close();
	return result;
}

/**
 * Change the password for a given user
 * @param username : user who request the change
 * @param newPassword : new password for the user
 * @return true in case of success
 */
bool Database::changePassword(QString username, QString newPassword) {
	qDebug () << username << newPassword;
	bool result = false;
	QString hashedUsername = hashUsername(std::move(username));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QSqlQuery saltQuery;
	saltQuery.prepare("SELECT salt FROM users WHERE username=:username");
	saltQuery.bindValue(":username", hashedUsername);

	QString salt = "";
	if (saltQuery.exec()) {
		if (saltQuery.isActive()) {
			if (saltQuery.first()) {
				salt = saltQuery.value(0).toString();
			}else{
				return false;
			}
		}
	}

	QString hashedNewPassword = hashPassword(std::move(newPassword), salt);

	QSqlQuery query;
	query.prepare("UPDATE users SET password=:newPassword WHERE username=:username");
	query.bindValue(":username", hashedUsername);
	query.bindValue(":newPassword", hashedNewPassword);

	if (!query.exec()) {
		qDebug() << "Error change password:\n" << query.lastError();
	} else
		result = true;

	db.close();
	return result;
}