//
// Created by andrea settimo on 2019-08-16.
//

#ifndef TEXTEDITOR_COMMONFUNCTIONS_H
#define TEXTEDITOR_COMMONFUNCTIONS_H
#include <QObject>
#include <QTcpSocket>
#include "../../../common/Constants.h"

bool readChunck(QTcpSocket *soc, QByteArray& result,qsizetype size);
bool readSpace(QTcpSocket *soc);
bool writeMessage(QTcpSocket *soc, QByteArray& message);
bool writeOkMessage(QTcpSocket *soc);
bool writeErrMessage(QTcpSocket *soc);
QByteArray convertionNumber(int number);
int readNumberFromSocket(QTcpSocket *socket);
bool readQString(QTcpSocket *soc,QString &in, int size);
QByteArray convertionQString(QString str);

#endif //TEXTEDITOR_COMMONFUNCTIONS_H
