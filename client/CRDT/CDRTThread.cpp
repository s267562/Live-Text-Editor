//
// Created by andrea settimo on 28/02/2020.
//

#include "CRDT/CDRTThread.h"

CDRTThread::CDRTThread(QObject *parent, CRDT *crdt):QThread(parent), crdt(crdt){}

void CDRTThread::run(){
    //editor->show();
    exec();
}