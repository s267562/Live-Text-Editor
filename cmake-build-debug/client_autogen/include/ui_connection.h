/********************************************************************************
** Form generated from reading UI file 'connection.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONNECTION_H
#define UI_CONNECTION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Connection
{
public:
    QWidget *centralWidget;
    QLineEdit *lineEdit;
    QPushButton *pushButton;
    QLabel *label;
    QMenuBar *menuBar;
    QMenu *menuConnessione;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *Connection)
    {
        if (Connection->objectName().isEmpty())
            Connection->setObjectName(QString::fromUtf8("Connection"));
        Connection->resize(237, 185);
        centralWidget = new QWidget(Connection);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setEnabled(true);
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(60, 40, 113, 21));
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(70, 70, 81, 32));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(60, 10, 51, 21));
        Connection->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Connection);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 237, 22));
        menuConnessione = new QMenu(menuBar);
        menuConnessione->setObjectName(QString::fromUtf8("menuConnessione"));
        Connection->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Connection);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        Connection->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(Connection);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        Connection->setStatusBar(statusBar);

        menuBar->addAction(menuConnessione->menuAction());

        retranslateUi(Connection);

        QMetaObject::connectSlotsByName(Connection);
    } // setupUi

    void retranslateUi(QMainWindow *Connection)
    {
        Connection->setWindowTitle(QApplication::translate("Connection", "Connection", nullptr));
        pushButton->setText(QApplication::translate("Connection", "Connetti", nullptr));
        label->setText(QApplication::translate("Connection", "Server:", nullptr));
        menuConnessione->setTitle(QApplication::translate("Connection", "Connessione", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Connection: public Ui_Connection {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONNECTION_H
