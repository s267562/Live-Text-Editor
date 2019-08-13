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
	if (!db.open())
		qDebug() << "Error opening DB";

	// Check DB was created empty
	QString checkEmptyQuery = "SELECT * FROM sqlite_master WHERE name = 'users'";

	// CREATE TABLE query string
	QString createTableQuery = "CREATE TABLE users ("
							   "username VARCHAR(256) PRIMARY KEY,"
							   "password VARCHAR(256),"
							   "salt VARCHAR(128),"
							   "avatar BLOB)";

	// Query executions
	QSqlQuery qry;
	if (qry.exec(checkEmptyQuery)) { // check db is empty
		if (qry.next())
			qDebug() << "DB is OK";
		else {
			qDebug() << "DB need table creation";
			// Query execution
			if (!qry.exec(createTableQuery))
				qDebug() << "Error creating table";
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
	QString toHash = password + salt;
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

	// Connect to db
	db.open();

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

	// Connect to db
	db.open();

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

	// Connect to db
	if (!db.open())
		qDebug() << "Cannot open DB";

	QSqlQuery query;
	query.prepare("UPDATE users SET avatar=:avatar WHERE username=:username");
	query.bindValue(":avatar", image);
	query.bindValue(":username", hashedUsername);

	if (!query.exec()) {
		qDebug() << "Error inserting image into table:\n" << query.lastError();
	} else
		result = true;
	//}
	db.close();
	return result;
}

/**
 * Get user's avatar
 * @param username : usrname of the user requested avatar
 * @return : QbyteArray of the avatar, may be NULL if user doesn't have an avatar yet
 */
QByteArray Database::getAvatar(QString username) {
	QString hashedUsername = hashUsername(std::move(username));

	// Connect to db
	if (!db.open())
		qDebug() << "Cannot open DB";

	QSqlQuery query;
	query.prepare("SELECT avatar FROM users WHERE username=:username");
	query.bindValue(":username", hashedUsername);

	if (!query.exec())
		qDebug() << "Error getting image from table:\n" << query.lastError();
	query.first();
	QByteArray avatar = query.value(0).toByteArray();
	db.close();
	return avatar;
}

