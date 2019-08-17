//
// Created by andrea settimo on 2019-08-16.
//

#include "commonFunctions.h"

bool readChunck(QTcpSocket *soc, QByteArray& result,qsizetype size){
    result = QByteArray();
    qsizetype read = 0, left = size;

    while (left != 0){
        if (soc->bytesAvailable() == 0 ){
            if (!soc->waitForReadyRead(TIMEOUT)){
                qDebug() << "Timeout! " << soc;
                return false;
            }
        }

        QByteArray resultI = soc->read(left);
        read = resultI.size();
        result.append(resultI);
        left -= read;
    }

    qDebug() << read << " " << left;
    return left == 0;
}

bool writeOkMessage(QTcpSocket *soc){
    if (soc == nullptr){
        return false;
    }

    soc->write(OK_MESSAGE);
    if (soc->waitForBytesWritten(TIMEOUT)){
        qDebug() << "Ok, scritto";
        return true;
    }else{
        qDebug() << "Ok, non scritto";
        return false;
    }
}

bool writeErrMessage(QTcpSocket *soc){
    if (soc == nullptr){
        return false;
    }

    soc->write(ERR_MESSAGE);
    if (soc->waitForBytesWritten(TIMEOUT)){
        qDebug() << "Err, scritto";
        return true;
    }else{
        qDebug() << "Err, non scritto";
        return false;
    }
}