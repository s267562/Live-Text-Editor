//
// Created by andrea settimo on 2019-08-16.
//

#ifndef TEXTEDITOR_COMMONFUNCTIONS_H
#define TEXTEDITOR_COMMONFUNCTIONS_H
#include <QObject>
#include <QTcpSocket>
#include "../../Utils/Constants.h"

bool readChunck(QTcpSocket *soc, QByteArray& result,qsizetype size);
bool readSpace(QTcpSocket *soc);
bool writeMessage(QTcpSocket *soc, QByteArray& message);
bool writeOkMessage(QTcpSocket *soc);
bool writeErrMessage(QTcpSocket *soc, QString type = "");
QByteArray convertionNumber(int number);
int readNumberFromSocket(QTcpSocket *socket);


#endif //TEXTEDITOR_COMMONFUNCTIONS_H
