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


private:
    Controller controller;
};

//! [1]
void TestGui::testGui()
{

    QApplication::setActiveWindow(&controller);

    QTest::qWaitForWindowExposed(controller.findChild<QMainWindow*>("Connection"));

    //QTest::keyClicks(controller.findChild<QLineEdit*>("serverIP"),"127.0.0.1");
    //QTest::keyClicks(controller.findChild<QLineEdit*>("serverPort"),"1234");
    QTest::mouseClick(controller.findChild<QMainWindow*>("Connection")->findChild<QPushButton*>("pushButton"),Qt::LeftButton);

    QTest::qWaitForWindowExposed(controller.findChild<QMainWindow*>("Login"));

    QTest::keyClicks(controller.findChild<QMainWindow*>("Login")->findChild<QLineEdit*>("username"),"u1");
    QTest::keyClicks(controller.findChild<QMainWindow*>("Login")->findChild<QLineEdit*>("password"),"u1");
    QTest::mouseClick(controller.findChild<QMainWindow*>("Login")->findChild<QPushButton*>("pushButton"),Qt::LeftButton);

    QTest::qWaitForWindowExposed(controller.findChild<QMainWindow*>("ShowFiles"));

    //QTest::mouseDClick(controller.findChild<QMainWindow*>("ShowFiles")->findChild<QListWidget*>("listWidget")->findChild<QListWidgetItem*>("file2"),Qt::LeftButton);


    QListWidgetItem* i = controller.findChild<QMainWindow*>("ShowFiles")->findChild<QListWidget*>("listWidget")->item(1);
    controller.findChild<QMainWindow*>("ShowFiles")->findChild<QListWidget*>("listWidget")->itemDoubleClicked(i);
    //controller.findChild<QListWidget*>("listWidget")->itemDoubleClicked(file);

    QTest::qWaitForWindowExposed(controller.findChild<QMainWindow*>("Editor"));

//    for(auto c:controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->children()){
//
//        qDebug() << c;
//
//
//
//    }
//
//    qDebug() << controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4)->text();
//
//    for(auto c:controller.findChild<QMainWindow*>("Editor")->findChild<QActionGroup*>("")->actions()){
//
//        qDebug() << c;
//
//    }

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

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Delete, Qt::NoModifier, 300);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);
    QTest::keyClicks(controller.findChild<QMainWindow*>("Editor")->centralWidget(), "Ciaovchreicaibvasdvhqeorgv", Qt::NoModifier, 20);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Enter, Qt::NoModifier, 100);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 300);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton); // 4 -> Center


    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 300);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);
    
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 300);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 300);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(5), Qt::LeftButton);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::NoModifier, 300);
    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(4), Qt::LeftButton);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Down, Qt::NoModifier, 300);


    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);

    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Up, Qt::ShiftModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Delete, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Delete, Qt::NoModifier, 300);
    QTest::keyClick(controller.findChild<QMainWindow*>("Editor")->centralWidget(), Qt::Key_Delete, Qt::NoModifier, 300);


    QTest::mouseClick(controller.findChild<QMainWindow*>("Editor")->findChild<QToolBar*>("")->findChildren<QToolButton*>("",Qt::FindDirectChildrenOnly).at(3), Qt::LeftButton);


    QTest::qWait(15000);

    //controller.s

    //controller.requestForFile("file2");


//    Messanger messanger;
//
//    CRDT crdt{}; // model
//    messanger.setCRDT( &crdt);
//
//    messanger.connectTo("127.0.0.1", "1234");
//    QString username = "u1";
//    messanger.logIn(username, "u1");                         /* NOTA: occorre registrare l'utente!! */
//
//    messanger.requestForFile("file2");
//
//    QString siteId = username;
//    crdt.setSiteId(siteId);
//
//    Q_INIT_RESOURCE(textEditor);
//
//    // view
//    Editor editor{siteId};
//
//    // controller
//    Controller controller{&crdt, &editor, &messanger};
//
//    editor.setController(&controller);

    /*const QRect availableGeometry = QApplication::desktop()->availableGeometry(&editor);
    editor.resize(availableGeometry.width() / 2, (availableGeometry.height() * 2) / 3);
    editor.move((availableGeometry.width() - editor.width()) / 2,
            (availableGeometry.height() - editor.height()) / 2);*/



}
//! [1]

//! [2]
QTEST_MAIN(TestGui)
#include "TestGui.moc"
//! [2]

#endif //TEXTEDITOR_TESTGUI_H
