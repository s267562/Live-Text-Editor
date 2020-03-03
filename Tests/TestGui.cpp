//
// Created by Eugenio Marinelli on 2020-02-29.
//

#ifndef TEXTEDITOR_TESTGUI_H
#define TEXTEDITOR_TESTGUI_H


#include <QtWidgets>
#include <QtTest/QtTest>
#include <clickablelabel.h>
#include <QtCore/qnamespace.h>
#include "../client/Controller.h"



class TestGui: public QObject
{

Q_OBJECT


private slots:
    void testGui();
    void process();


private:
    Controller controller;
    Controller controller2;

    int n=1;

    void logClient(Controller &controller, QString username, QString password);
    void generalAction(Controller &controller);


};


void TestGui::process(){
    if(n==1){
        generalAction(controller);
    }else{
        generalAction(controller2);
    }

}

//! [1]
void TestGui::testGui()
{

    QApplication::setActiveWindow(&controller);

    QApplication::setActiveWindow(&controller2);

    logClient(controller, "u1","u1");

    logClient(controller2, "u2","u2");

    QThread* t1=new QThread();
    controller.moveToThread(t1);

    connect(t1,SIGNAL(started()),this,SLOT(process()));

    QThread* t2=new QThread();
    controller2.moveToThread(t2);

    connect(t2,SIGNAL(started()),this,SLOT(process()));

    n=1;
    t1->start();

    // generalAction(controller);

    n=2;
    t2->start();

    // generalAction(controller);



}
//! [1]


void TestGui::logClient(Controller &controller, QString username, QString password){

    QTest::qWaitForWindowExposed(controller.findChild<QMainWindow*>("Connection"));

    QTest::mouseClick(controller.findChild<QMainWindow*>("Connection")->findChild<QPushButton*>("pushButton"),Qt::LeftButton);

    QTest::qWaitForWindowExposed(controller.findChild<QMainWindow*>("Login"));

    QTest::keyClicks(controller.findChild<QMainWindow*>("Login")->findChild<QLineEdit*>("username"),username);
    QTest::keyClicks(controller.findChild<QMainWindow*>("Login")->findChild<QLineEdit*>("password"),password);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Login")->findChild<QPushButton*>("pushButton"),Qt::LeftButton);

    QTest::qWaitForWindowExposed(controller.findChild<QMainWindow*>("ShowFiles"));


    QListWidgetItem* i = controller.findChild<QMainWindow*>("ShowFiles")->findChild<QListWidget*>("listWidget")->item(1);
    controller.findChild<QMainWindow*>("ShowFiles")->findChild<QListWidget*>("listWidget")->itemDoubleClicked(i);

    QTest::qWaitForWindowExposed(controller.findChild<QMainWindow*>("Editor"));
}

void TestGui::generalAction(Controller &controller) {
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);

    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "Ciaovfsnfvsotnveht gveaovghqeovhgaergv", Qt::NoModifier, 20);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);

    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "Ciaovchreicaibvasdvhqeorgv", Qt::NoModifier, 20);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);

    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "Ciaovfs nfvsotnveht gveaovghqeov hgaergv", Qt::NoModifier, 20);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);

    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "Cwevwevkvnroegv", Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 100);
    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "Ciaovchreicaibvasdvhqeorgv", Qt::NoModifier, 20);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Delete, Qt::NoModifier, 100);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);
    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "Ciaovchreicaibvasdvhqeorgv", Qt::NoModifier, 20);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 100);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton); // 4 -> Center


    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 100);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 100);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 100);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(5), Qt::LeftButton);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 100);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 100);


    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Delete, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Delete, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Delete, Qt::NoModifier, 100);


    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(3), Qt::LeftButton);
}

//! [2]
QTEST_MAIN(TestGui)
#include "TestGui.moc"
//! [2]

#endif //TEXTEDITOR_TESTGUI_H