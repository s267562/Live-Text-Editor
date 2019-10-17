//
// Created by andrea settimo on 2019-08-16.
//

#include <QDataStream>
#include "commonFunctions.h"

bool readChunck(QTcpSocket *soc, QByteArray& result,qsizetype size){
    result = QByteArray();
    qsizetype read = 0, left = size;

    while (left != 0){
        if (soc->bytesAvailable() == 0 ){
            if (!soc->waitForReadyRead(TIMEOUT)){
                qDebug() << "server/Networking/common/commonFunctions.cpp - readChunck()     Timeout! " << soc;
                qDebug() << ""; // newLine
                return false;
            }
        }

        QByteArray resultI = soc->read(left);
        read = resultI.size();
        result.append(resultI);
        left -= read;
    }
    return left == 0;
}

bool readSpace(QTcpSocket *soc){
    if (soc->bytesAvailable() == 0) {
        if (!soc->waitForReadyRead(TIMEOUT)) {
            qDebug() << "server/Networking/common/commonFunctions.cpp - readSpace()     Timeout! " << soc;
            qDebug() << ""; // newLine
            return false;
        }
    }
    soc->read(1);
    return true;
}

bool writeMessage(QTcpSocket *soc, QByteArray& message){
    soc->write(message);
    if (soc->waitForBytesWritten(TIMEOUT)){
        return true;
    }else{
        return false;
    }
}

bool writeOkMessage(QTcpSocket *soc){
    if (soc == nullptr){
        return false;
    }

    soc->write(OK_MESSAGE);
    if (soc->waitForBytesWritten(TIMEOUT)){
        qDebug() << "server/Networking/common/commonFunctions.cpp - writeOkMessage()     \"Ok\" scritto";
        qDebug() << ""; // newLine
        return true;
    }else{
        qDebug() << "server/Networking/common/commonFunctions.cpp - writeOkMessage()     \"Ok\" non scritto";
        qDebug() << ""; // newLine
        return false;
    }
}

bool writeErrMessage(QTcpSocket *soc, QString type){
    if (soc == nullptr){
        return false;
    }

    QByteArray message;
    message.append(ERR_MESSAGE);

    if (type != ""){
        message.append(" " + type.toUtf8());
    }

    soc->write(message);
    if (soc->waitForBytesWritten(TIMEOUT)){
        qDebug() << "server/Networking/common/commonFunctions.cpp - writeErrMessage()     \"Err\" scritto";
        qDebug() << ""; // newLine
        return true;
    }else{
        qDebug() << "server/Networking/common/commonFunctions.cpp - writeErrMessage()     \"Err\" non scritto";
        qDebug() << ""; // newLine
        return false;
    }
}

QByteArray convertionNumber(int number){
    QByteArray numberResult;
    QDataStream outNumberResult(&numberResult, QIODevice::WriteOnly);
    outNumberResult << number;
    return numberResult;
}

int readNumberFromSocket(QTcpSocket *socket){
    QDataStream outNumberResult(socket);
    int result;
    outNumberResult >> result;
    return result;
}

bool readQString(QTcpSocket *soc, QString &in, int size){
    QByteArray byteArray;
    if (!readChunck(soc, byteArray, size)) {
        return false;
    }
    qDebug() << byteArray;
    in = QString::fromUtf16(reinterpret_cast<const ushort*>(byteArray.data()), size/2);
    return true;
}

QByteArray convertionQString(QString str){
    QByteArray returnValue(reinterpret_cast<const char *>(str.utf16()), str.size() * 2);
    return returnValue;
}