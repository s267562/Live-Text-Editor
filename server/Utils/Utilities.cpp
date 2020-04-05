/**
 * Created by Davide Sordi
 * Using CLion
 * Date: 26/03/2020
 * Time: 14.30
 * 
 * Class: Utilities
 * File: Utilities.cpp	
 * Project: textEditor	
 */


#include <QtCore/QFile>
#include <QDebug>
#include <QtCore/QDir>

QString directories[] = {"saveData", "backup1", "backup2"};

/**
 * This function delete a file from the server and also from all the backup directories
 * @param filename
 * @return true if deleted, false in case of error
 */
bool deleteFileSave(QString filename) {
	bool result = true;

	for (QString dir : directories) {
		QFile savefile(dir + "/" + filename + ".json");
		if (savefile.exists()) {
			if (!savefile.remove()) {
				result = false;
				qDebug() << "Error deleting: " << savefile.fileName();
			} else {
				qDebug() << "Deleted: " + filename;
			}
		}
	}
	return result;
}

/**
 * This function is used for renaming all saved files (including backup ones)
 * @return true if renamed, false in case of error
 */
bool renameFileSave(QString oldFilename, QString newFilename) {
	bool result = true;

	for (QString dir : directories) {
		QFile savefile(dir + "/" + oldFilename + ".json");
		if (savefile.exists()) {
			if (!savefile.rename(dir + "/" + newFilename + ".json")) {
				result = false;
				qDebug() << "Error renaming: " << savefile.fileName();
			} else {
				qDebug() << "Renamed '" + oldFilename + "' into '" + newFilename + "'";
			}
		}
	}
	return result;
}

/**
 * Check and create if needed folders for file saving and backups
 * Folders: files - backup1 - backup2
 * @return
 */
bool checkAndCreateSaveDir() {
	bool result = true;

	// Check existance
	for (const QString &name : directories) {
		QDir dir(name);
		if (!dir.exists())
			if (QDir().mkdir(name))
				qDebug() << name << " created";
			else {
				qDebug() << "Error creating " << name << " check you have permissions!!!";
				result = false;
			}
		else
			qDebug() << name << " exists";
	}
	return result;
}

/**
 * Make a backup of the saved files into backup directory
 * @param primary true if backup is a primary one (more frequent) false else (less frequent)
 * @return true if no errors occurred
 */
void backupFile(QString filename, bool primary, bool binary) {
	QDir dataDir(directories[0]);
	QDir backupDir;
	if (primary)
		backupDir = QDir(directories[1]);
	else
		backupDir = QDir(directories[2]);

	if (binary)
		filename = filename + ".dat";
	else filename = filename + ".json";

	qDebug() << "File: " + filename + " " + primary;
	if (backupDir.exists(filename)) {
		backupDir.remove(filename);
		qDebug() << "--Removed";
	}
	QFile::copy(dataDir.dirName() + "/" + filename, backupDir.dirName() + "/" + filename);
	qDebug() << "++Copied";
}