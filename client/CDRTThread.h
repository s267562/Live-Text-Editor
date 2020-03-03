//
// Created by andrea settimo on 28/02/2020.
//

#ifndef TEXTEDITOR_CDRTTHREAD_H
#define TEXTEDITOR_CDRTTHREAD_H

#include <QThread>
#include "CRDT.h"
#include "Controller.h"
#include "CRDT.h"

class Controller;
class CRDT;

class CDRTThread: public QThread {
    CRDT *crdt = nullptr;

public:
    explicit CDRTThread(QObject *parent = nullptr, CRDT *crdt = nullptr);
    void run();
    void show();
};


#endif //TEXTEDITOR_CDRTTHREAD_H
