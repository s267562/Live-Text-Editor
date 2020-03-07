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
 * Constructor for the server
 */
Database::Database() {
	connectionName = "conn-MAIN";
	db = QSqlDatabase::addDatabase("QSQLITE",connectionName);
	db.setDatabaseName("database.sqlite");
	this->initTables();
}

/**
 * Constructor specifying a connection name based on thread ID
 * @param threadID
 */
Database::Database(QString threadID) {
	connectionName ="conn-" + threadID;
	db = QSqlDatabase::addDatabase("QSQLITE",connectionName);
	db.setDatabaseName("database.sqlite");

	// We expect no need of tables initialization
}

/**
 * This function creates tables if necessary
 */
void Database::initTables() {

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
			"PRIMARY KEY (fileID, username));";

	QString createTableQuerySharing =
			"CREATE TABLE sharing("
			"owner VARCHAR(256),"
			"fileID VARCHAR(256),"
			"sharedToUser VARCHAR(256),"
			"read INT,"
			"write INT,"
			"PRIMARY KEY(owner, fileID, sharedToUser));";


	// Query executions
	QSqlQuery qry(db);
	if (qry.exec(checkEmptyQuery)) { // check db is empty
		if (qry.next())
			qDebug() << "DB is OK";
		else {
			qDebug() << "DB need table creation";
			// Query execution
			if (!qry.exec(createTableQuery))
				qDebug() << "Error creating table users\n" << qry.lastError();
			if (!qry.exec(createTableQueryFiles))
				qDebug() << "Error creating table files\n" << qry.lastError();
			if (!qry.exec(createTableQuerySharing))
				qDebug() << "Error creating table sharing\n" << qry.lastError();
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

	// Start a new transaction
	QSqlDatabase::database().transaction();

	QString querySelect = "SELECT * FROM users WHERE username='" + hashedUsername + "'";
	QSqlQuery checkUsernameQuery(db);

	if (checkUsernameQuery.exec(querySelect)) {
		if (checkUsernameQuery.isActive()) {
			if (!checkUsernameQuery.next()) {
				// Username free for use, we can register the user

				QString salt = generateSalt();
				QString hashedPassword = hashPassword(std::move(password), salt);

				QSqlQuery qry(db);
				qry.prepare("INSERT INTO users ("
							"username,"
							"password,"
							"salt)"
							"VALUES (?,?,?);");

				qry.addBindValue(hashedUsername);
				qry.addBindValue(hashedPassword);
				qry.addBindValue(salt);

				if (!qry.exec())
					qDebug() << "Error inserting query\n" << qry.lastError();
				else result = true;
			}
		}
	}

	// Commit rollback for the transaction
	if (result)
		QSqlDatabase::database().commit();
	else
		QSqlDatabase::database().rollback();
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

	QSqlQuery authenticationQuery(db);
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

	QSqlQuery query(db);
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

	QSqlQuery query(db);
	query.prepare("SELECT avatar FROM users WHERE username=:username");
	query.bindValue(":username", hashedUsername);

	if (!query.exec())
		qDebug() << "Error getting image from table:\n" << query.lastError();
	query.first();
	QByteArray avatar = query.value(0).toByteArray();
	if (avatar.isNull())
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

	// Start transaction
	QSqlDatabase::database().transaction();

	// Check username is valid
	QSqlQuery queryCheckUsername(db);
	queryCheckUsername.prepare("SELECT * FROM users WHERE username=:username");
	queryCheckUsername.bindValue(":username", hashedUsername);

	if (!queryCheckUsername.exec()) {
		qDebug() << "Error getting username from table:\n" << queryCheckUsername.lastError();
	} else {
		// Username is valid, register file id
		if (queryCheckUsername.first()) {
			QSqlQuery queryRegisterFile(db);
			queryRegisterFile.prepare("INSERT INTO files ("
									  "fileID,"
									  "username)"
									  "VALUES (?,?)");
			queryRegisterFile.addBindValue(fileID);
			queryRegisterFile.addBindValue(hashedUsername);

			if (!queryRegisterFile.exec())
				qDebug() << "Error registering new file:\n" << queryRegisterFile.lastError();
			else
				result = true;
		}
	}
	// Commit rollback for the transaction
	if (result)
		QSqlDatabase::database().commit();
	else
		QSqlDatabase::database().rollback();
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

	QSqlQuery queryCheckOwner(db);
	queryCheckOwner.prepare("SELECT * FROM files WHERE fileID=:fileID AND username=:username");
	queryCheckOwner.bindValue(":fileID", fileID);
	queryCheckOwner.bindValue(":username", hashedUsername);

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
 * @param fileID : file to share
 * @param owner : user who own the file
 * @param username : user who need permissions
 * @param permission : ENUM type for adding permission READ or WRITE
 * @return true if permissions modified, false else
 */
bool Database::addPermission(QString fileID, QString owner, QString username) {
	bool result = false;
	//QString hashedUsername = hashUsername(username);
	QString hashedOwner = hashUsername(owner);

	// if the user is already the owner of the file we dont need a new permission
	if (isOwner(fileID, username)) {
		db.close();
		return false;
	}

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	// Start transaction
	QSqlDatabase::database().transaction();

	// check if this file is already shared with this username
	QSqlQuery queryCheckAlreadyShared(db);
	queryCheckAlreadyShared.prepare(
			"SELECT * FROM sharing WHERE owner=:owner AND fileID=:fileID AND sharedToUser=:username");
	queryCheckAlreadyShared.bindValue(":owner", hashedOwner);
	queryCheckAlreadyShared.bindValue(":fileID", fileID);
	queryCheckAlreadyShared.bindValue(":username", username);

	if (!queryCheckAlreadyShared.exec()) {
		qDebug() << "Error checking permission:\n" << queryCheckAlreadyShared.lastError();
	} else {
		// File is already shared with this user
		if (queryCheckAlreadyShared.first()) {
			qDebug() << "File is already shared with this user";
		} else {
			// User need permission
			QSqlQuery queryAddPermission(db);
			queryAddPermission.prepare(
					"INSERT INTO sharing (owner,fileID,sharedToUser) VALUES (:owner, :fileID, :sharedToUser)");
			queryAddPermission.bindValue(":owner", hashedOwner);
			queryAddPermission.bindValue(":fileID", fileID);
			queryAddPermission.bindValue(":sharedToUser", username);

			if (!queryAddPermission.exec()) {
				qDebug() << "Error adding file permission for the user :\n" << queryAddPermission.lastError();
			} else
				result = true;
		}
	}

	// Commit rollback for the transaction
	if (result)
		QSqlDatabase::database().commit();
	else
		QSqlDatabase::database().rollback();
	db.close();
	return result;
}

/**
 * Retrieve all filenames owned by and shared to the given user
 * @param username : username to get files owned
 * @return : list of filenames (QString,bool) --------- return list with single object "DBERROR,false" in case of error
 *  boolean value indicates the ownership for a file
 */
std::map<QString, bool> Database::getFiles(QString username) {
	QString hashedUsername = hashUsername(username);
	std::map<QString, bool> userFiles;
	bool result = false;

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		userFiles["DBERROR"] = false;
		return userFiles;
	}

	// Start transaction
	QSqlDatabase::database().transaction();

	QSqlQuery getUserOwnedFiles(db);
	getUserOwnedFiles.prepare("SELECT fileID FROM files WHERE username=:username ");
	getUserOwnedFiles.bindValue(":username", hashedUsername);

	if (!getUserOwnedFiles.exec()) {
		qDebug() << "Error getting files for the user: " << username << "\n" << getUserOwnedFiles.lastError();
		db.close();
		userFiles["DBERROR"] = false;
		//return userFiles;
	} else {
		while (getUserOwnedFiles.next()) {
			userFiles[getUserOwnedFiles.value(0).toString()] = true;
		}

		QSqlQuery getUserWriteFiles(db);
		getUserWriteFiles.prepare("SELECT fileID FROM sharing WHERE sharedToUser=:sharedToUser");
		getUserWriteFiles.bindValue(":sharedToUser", username);

		if (!getUserWriteFiles.exec()) {
			qDebug() << "Error getting files for the user: " << username << "\n" << getUserWriteFiles.lastError();
			db.close();
			userFiles.clear();
			userFiles["DBERROR"] = false;
			//return userFiles;
		} else {
			while (getUserWriteFiles.next()) {
				userFiles[getUserWriteFiles.value(0).toString()] = true;
			}
			result = true;
		}
	}

	// Commit rollback for the transaction
	if (result)
		QSqlDatabase::database().commit();
	else
		QSqlDatabase::database().rollback();
	db.close();
	return userFiles;
}

/**
 * Change the username of a given user
 * @param oldUsername : username to be changed
 * @param newUsername : new username to insert in DB
 * @return true in case of success.
 */
bool Database::changeUsername(QString oldUsername, QString newUsername) {
	bool result = true;
	QString hashedOldUsername = hashUsername(oldUsername);
	QString hashedNewUsername = hashUsername(newUsername);

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	// Start transaction
	QSqlDatabase::database().transaction();

	QSqlQuery queryUsers(db);
	queryUsers.prepare("UPDATE users SET username=:newUsername WHERE username=:oldUsername");
	queryUsers.bindValue(":oldUsername", hashedOldUsername);
	queryUsers.bindValue(":newUsername", hashedNewUsername);

	if (!queryUsers.exec()) {
		qDebug() << "Error change username in table USERS:\n" << queryUsers.lastError();
		QSqlDatabase::database().rollback();
		db.close();
		return false;
	}

	QSqlQuery queryFiles(db);
	queryFiles.prepare("UPDATE files SET username=:newUsername WHERE username=:oldUsername");
	queryFiles.bindValue(":oldUsername", hashedOldUsername);
	queryFiles.bindValue(":newUsername", hashedNewUsername);

	if (!queryFiles.exec()) {
		qDebug() << "Error change username in table FILES:\n" << queryFiles.lastError();
		QSqlDatabase::database().rollback();
		db.close();
		return false;
	}

	QSqlQuery querySharing1(db);
	querySharing1.prepare("UPDATE sharing SET owner=:newUsername WHERE owner=:oldUsername");
	querySharing1.bindValue(":oldUsername", hashedOldUsername);
	querySharing1.bindValue(":newUsername", hashedNewUsername);

	if (!querySharing1.exec()) {
		qDebug() << "Error change username in table SHARING OWNER:\n" << querySharing1.lastError();
		QSqlDatabase::database().rollback();
		db.close();
		return false;
	}

	QSqlQuery querySharing2(db);
	querySharing2.prepare("UPDATE sharing SET sharedToUser=:newUsername WHERE sharedToUser=:oldUsername");
	querySharing2.bindValue(":oldUsername", oldUsername);
	querySharing2.bindValue(":newUsername", newUsername);

	if (!querySharing2.exec()) {
		qDebug() << "Error change username in table SHARING SHARED:\n" << querySharing2.lastError();
		QSqlDatabase::database().rollback();
		db.close();
		return false;
	}

	QSqlDatabase::database().commit();
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
	qDebug() << username << newPassword;
	bool result = false;
	QString hashedUsername = hashUsername(std::move(username));

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	// Start transaction
	QSqlDatabase::database().transaction();

	QSqlQuery saltQuery(db);
	saltQuery.prepare("SELECT salt FROM users WHERE username=:username");
	saltQuery.bindValue(":username", hashedUsername);

	QString salt = "";
	if (saltQuery.exec()) {
		if (saltQuery.isActive()) {
			if (saltQuery.first()) {
				salt = saltQuery.value(0).toString();
			} else {
				QSqlDatabase::database().rollback();
				db.close();
				return false;
			}
		}
	}

	QString hashedNewPassword = hashPassword(std::move(newPassword), salt);

	QSqlQuery query(db);
	query.prepare("UPDATE users SET password=:newPassword WHERE username=:username");
	query.bindValue(":username", hashedUsername);
	query.bindValue(":newPassword", hashedNewPassword);

	if (!query.exec()) {
		qDebug() << "Error change password:\n" << query.lastError();
		QSqlDatabase::database().rollback();
	} else {
		result = true;
		QSqlDatabase::database().commit();
	}

	db.close();
	return result;
}

/**
 * Retrieve the list of user to whom the file is shared
 * @param filename
 * @return List of users
 */
QStringList Database::getUsers(QString filename) {
	QStringList users;

	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		users.push_back("Error");
		return users;
	}

	QSqlQuery getUsers(db);
	getUsers.prepare("SELECT sharedToUser FROM sharing WHERE fileID=:fileID ");
	getUsers.bindValue(":fileID", filename);

	if (!getUsers.exec()) {
		qDebug() << "Error getting user list for: " << filename << "\n" << getUsers.lastError();
		db.close();
		users.push_back("Error");
		return users;
	}

	while (getUsers.next()) {
		users.push_back(getUsers.value(0).toString());
	}
	db.close();
	return users;
}

/**
 * Function for changing the name of a file
 * @param oldFilename
 * @param newFilename
 * @return
 */
bool Database::changeFileName(QString oldFilename, QString newFilename) {
	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	// Start transaction
	QSqlDatabase::database().transaction();

	QSqlQuery filesUpdate(db);
	filesUpdate.prepare("UPDATE files SET fileID=:fileID1 WHERE fileID=:fileID2");
	filesUpdate.bindValue(":fileID1", newFilename);
	filesUpdate.bindValue(":fileID2", oldFilename);

	if (!filesUpdate.exec()) {
		qDebug() << "Error getting user list for: " << oldFilename << "\n" << filesUpdate.lastError();
		QSqlDatabase::database().rollback();
		db.close();
		return false;
	}

	QSqlQuery sharingUpdate(db);
	sharingUpdate.prepare("UPDATE sharing SET fileID=:fileID1 WHERE fileID=:fileID2");
	sharingUpdate.bindValue(":fileID1", newFilename);
	sharingUpdate.bindValue(":fileID2", oldFilename);

	if (!sharingUpdate.exec()) {
		qDebug() << "Error getting user list for: " << oldFilename << "\n" << sharingUpdate.lastError();
		QSqlDatabase::database().rollback();
		db.close();
		return false;
	}

	QSqlDatabase::database().commit();
	db.close();
	return true;
}

/**
 * Remove the permission of a given user for a given filename
 * @param filename
 * @param username
 * @return
 */
bool Database::removePermission(QString filename, QString username) {
	// DB opening
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	QSqlQuery removeUser(db);
	removeUser.prepare("DELETE FROM sharing WHERE fileID=:fileID AND sharedToUser=:sharedToUser");
	removeUser.bindValue(":fileID", filename);
	removeUser.bindValue(":sharedToUser", username);

	if (!removeUser.exec()) {
		qDebug() << "Error getting user list for: " << filename << "\n" << removeUser.lastError();
		db.close();
		return false;
	}

	db.close();
	return true;
}

/**
 * Delete a file from the server
 * @param filename
 * @return
 */
bool Database::deleteFile(QString filename) {
	if (!db.open()) {
		qDebug() << "Error opening DB";
		return false;
	}

	// Start transaction
	QSqlDatabase::database().transaction();

	QSqlQuery removeUser(db);
	removeUser.prepare("DELETE FROM sharing WHERE fileID=:fileID");
	removeUser.bindValue(":fileID", filename);

	if (!removeUser.exec()) {
		qDebug() << "Error getting user list for: " << filename << "\n" << removeUser.lastError();
		QSqlDatabase::database().rollback();
		db.close();
		return false;
	}

	QSqlQuery removeFile(db);
	removeFile.prepare("DELETE FROM files WHERE fileID=:fileID");
	removeFile.bindValue(":fileID", filename);

	if (!removeFile.exec()) {
		qDebug() << "Error getting user list for: " << filename << "\n" << removeFile.lastError();
		QSqlDatabase::database().rollback();
		db.close();
		return false;
	}

	QSqlDatabase::database().commit();
	db.close();
	return true;
}