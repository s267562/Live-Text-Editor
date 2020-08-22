/**
 * Created by Davide Sordi
 * Using CLion
 * Date: 26/03/2020
 * Time: 14.30
 *
 * Class: Utilities
 * File: Utilities.h
 * Project: textEditor
 */

#ifndef TEXTEDITOR_UTILITIES_H
#define TEXTEDITOR_UTILITIES_H

#include <QtCore/QString>


bool deleteFileSave(QString filename);

bool renameFileSave(QString oldFilename, QString newFilename);

bool checkAndCreateSaveDir();

void backupFile(QString filename, bool primary = true, bool binary = false);

#endif //TEXTEDITOR_UTILITIES_H
