//
// Created by andrea settimo on 28/02/2020.
//

#include "CDRTThread.h"

CDRTThread::CDRTThread(QObject *parent, CRDT *crdt):QThread(parent), crdt(crdt){}

void CDRTThread::run(){
    try {
        exec();
    }catch (...) {
        //std::cout << "something went wrong";
        QMetaObject::invokeMethod(parent(), "reciveExternalErrorOrException", Qt::QueuedConnection);
    }
}