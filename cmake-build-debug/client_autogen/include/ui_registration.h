/********************************************************************************
** Form generated from reading UI file 'registration.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTRATION_H
#define UI_REGISTRATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>
#include "../../../client/ui/clickablelabel.h"

QT_BEGIN_NAMESPACE

class Ui_Registration
{
public:
    QWidget *centralWidget;
    ClickableLabel *label;
    QToolButton *toolButton;
    QGroupBox *groupBox;
    QLineEdit *lineEdit;
    QLineEdit *lineEdit_2;
    QLabel *label_2;
    QLabel *label_3;
    QPushButton *pushButton_registration;
    QPushButton *pushButton_login;
    QMenuBar *menuBar;
    QMenu *menuREgistration;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *Registration)
    {
        if (Registration->objectName().isEmpty())
            Registration->setObjectName(QString::fromUtf8("Registration"));
        Registration->resize(400, 300);
        centralWidget = new QWidget(Registration);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        label = new ClickableLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 40, 80, 80));
        label->setPixmap(QPixmap(QString::fromUtf8(":/icons/user_icon.jpg")));
        label->setScaledContents(true);
        toolButton = new QToolButton(centralWidget);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));
        toolButton->setGeometry(QRect(44, 140, 51, 21));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(130, 30, 241, 141));
        QFont font;
        font.setPointSize(14);
        groupBox->setFont(font);
        lineEdit = new QLineEdit(groupBox);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(20, 30, 200, 25));
        lineEdit_2 = new QLineEdit(groupBox);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));
        lineEdit_2->setGeometry(QRect(20, 89, 200, 25));
        lineEdit_2->setEchoMode(QLineEdit::Password);
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 8, 71, 16));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 67, 71, 16));
        pushButton_registration = new QPushButton(centralWidget);
        pushButton_registration->setObjectName(QString::fromUtf8("pushButton_registration"));
        pushButton_registration->setGeometry(QRect(60, 190, 113, 32));
        pushButton_login = new QPushButton(centralWidget);
        pushButton_login->setObjectName(QString::fromUtf8("pushButton_login"));
        pushButton_login->setGeometry(QRect(210, 190, 113, 32));
        Registration->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Registration);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 22));
        menuREgistration = new QMenu(menuBar);
        menuREgistration->setObjectName(QString::fromUtf8("menuREgistration"));
        Registration->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Registration);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        Registration->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(Registration);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        Registration->setStatusBar(statusBar);

        menuBar->addAction(menuREgistration->menuAction());

        retranslateUi(Registration);

        QMetaObject::connectSlotsByName(Registration);
    } // setupUi

    void retranslateUi(QMainWindow *Registration)
    {
        Registration->setWindowTitle(QApplication::translate("Registration", "Registrazione", nullptr));
        label->setText(QString());
        toolButton->setText(QApplication::translate("Registration", "Rimuovi", nullptr));
        groupBox->setTitle(QString());
        label_2->setText(QApplication::translate("Registration", "Username", nullptr));
        label_3->setText(QApplication::translate("Registration", "Password", nullptr));
        pushButton_registration->setText(QApplication::translate("Registration", "Registrati", nullptr));
        pushButton_login->setText(QApplication::translate("Registration", "Log In", nullptr));
        menuREgistration->setTitle(QApplication::translate("Registration", "Registrazione", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Registration: public Ui_Registration {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTRATION_H
