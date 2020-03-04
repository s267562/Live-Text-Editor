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
    void initTestCase();
    void testGuiAlignments();
    void testGui();



private:
    Controller controller;
    Controller controller2;

    void logClient(Controller &controller, QString username, QString password);
    void generalAction(Controller &controller);
    void testingDelete(Controller &controller);
    void testingAlignmentFirstAndLastLine(Controller &controller);

};

//! [1]
void TestGui::testGui()
{
    /*
    generalAction(controller);

    generalAction(controller2);*/
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


void TestGui::testingDelete(Controller &controller) {
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);

}

void TestGui::testingAlignmentFirstAndLastLine(Controller &controller) {
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);
    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "This is an alignment test", Qt::NoModifier, 20);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 50);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 50);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 50);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(3), Qt::LeftButton);
    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "This is an other alignment test", Qt::NoModifier, 20);
}

void TestGui::testGuiAlignments() {

    this->testingAlignmentFirstAndLastLine(controller);

    std::vector<std::vector<Character>> strC1 = this->controller.crdt->getStructure();
    std::vector<std::vector<Character>> strC2 = this->controller2.crdt->getStructure();

    std::vector<std::pair<Character,int>> styC1 = this->controller.crdt->getStyle();
    std::vector<std::pair<Character,int>> styC2 = this->controller2.crdt->getStyle();

    QCOMPARE(strC1,strC2);
    QCOMPARE(styC1,styC2);

    QCOMPARE(viewport,this->controller2.editor->centralWidget()->findChild<QWidget*>("viewport"));

}

void TestGui::initTestCase() {

    QApplication::setActiveWindow(&controller);

    QApplication::setActiveWindow(&controller2);

    logClient(controller, "u1","u1");

    logClient(controller2, "u2","u2");
}




//! [2]
QTEST_MAIN(TestGui);
#include "TestGui.moc"
//! [2]

#endif //TEXTEDITOR_TESTGUI_H
