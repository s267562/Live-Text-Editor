/********************************************************************************
** Form generated from reading UI file 'registration.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTRATION_H
#define UI_REGISTRATION_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>
#include "clickablelabel.h"

QT_BEGIN_NAMESPACE

class Ui_Registration
{
public:
    QWidget *centralWidget;
    QWidget *centralWidget_2;
    QWidget *widget;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_7;
    QLineEdit *username;
    QLineEdit *password;
    QPushButton *pushButton_login;
    QPushButton *pushButton_registration;
    ClickableLabel *label;
    QToolButton *toolButton;
    QLabel *error;
    QMenuBar *menuBar;
    QMenu *menuREgistration;

    void setupUi(QMainWindow *Registration)
    {
        if (Registration->objectName().isEmpty())
            Registration->setObjectName(QString::fromUtf8("Registration"));
        Registration->resize(600, 600);
        Registration->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        centralWidget = new QWidget(Registration);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setStyleSheet(QString::fromUtf8("QWidget{\n"
"	background-color: rgb(233, 230, 234);\n"
"}"));
        centralWidget_2 = new QWidget(centralWidget);
        centralWidget_2->setObjectName(QString::fromUtf8("centralWidget_2"));
        centralWidget_2->setGeometry(QRect(0, 0, 800, 800));
        centralWidget_2->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        widget = new QWidget(centralWidget_2);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(70, 100, 441, 421));
        widget->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);"));
        label_4 = new QLabel(widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 160, 71, 31));
        label_5 = new QLabel(widget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 220, 71, 31));
        label_7 = new QLabel(widget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 0, 241, 41));
        label_7->setStyleSheet(QString::fromUtf8("font-size: 32px;"));
        username = new QLineEdit(widget);
        username->setObjectName(QString::fromUtf8("username"));
        username->setGeometry(QRect(100, 160, 331, 31));
        password = new QLineEdit(widget);
        password->setObjectName(QString::fromUtf8("password"));
        password->setGeometry(QRect(100, 220, 331, 31));
        password->setEchoMode(QLineEdit::Password);
        pushButton_login = new QPushButton(widget);
        pushButton_login->setObjectName(QString::fromUtf8("pushButton_login"));
        pushButton_login->setGeometry(QRect(10, 340, 421, 41));
        pushButton_login->setStyleSheet(QString::fromUtf8("QPushButton{  \n"
"background-color: black;  \n"
"border: none;\n"
"color: white;\n"
"text-align: center;\n"
"margin: 4px 2px;\n"
"opacity: 0.6;\n"
"text-decoration: none;\n"
"}\n"
"QPushButton:hover{\n"
"background-color: white;\n"
"border: 1px solid black;\n"
"color : black;}"));
        pushButton_registration = new QPushButton(widget);
        pushButton_registration->setObjectName(QString::fromUtf8("pushButton_registration"));
        pushButton_registration->setGeometry(QRect(10, 280, 421, 41));
        pushButton_registration->setStyleSheet(QString::fromUtf8("QPushButton{  \n"
"background-color: red;  \n"
"border: none;\n"
"color: white;\n"
"text-align: center;\n"
"margin: 4px 2px;\n"
"opacity: 0.6;\n"
"text-decoration: none;\n"
"}\n"
"QPushButton:hover{\n"
"margin: 2px 1px;\n"
"}"));
        label = new ClickableLabel(widget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(180, 60, 80, 80));
        label->setPixmap(QPixmap(QString::fromUtf8(":/icons/user_icon.jpg")));
        label->setScaledContents(true);
        toolButton = new QToolButton(widget);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));
        toolButton->setGeometry(QRect(270, 120, 51, 21));
        error = new QLabel(widget);
        error->setObjectName(QString::fromUtf8("error"));
        error->setGeometry(QRect(10, 390, 421, 20));
        error->setStyleSheet(QString::fromUtf8("color: rgb(255, 0, 0);"));
        error->setAlignment(Qt::AlignCenter);
        error->setIndent(0);
        Registration->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(Registration);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 22));
        menuREgistration = new QMenu(menuBar);
        menuREgistration->setObjectName(QString::fromUtf8("menuREgistration"));
        Registration->setMenuBar(menuBar);

        menuBar->addAction(menuREgistration->menuAction());

        retranslateUi(Registration);

        QMetaObject::connectSlotsByName(Registration);
    } // setupUi

    void retranslateUi(QMainWindow *Registration)
    {
        Registration->setWindowTitle(QApplication::translate("Registration", "Registrazione", nullptr));
        label_4->setText(QApplication::translate("Registration", "Username", nullptr));
        label_5->setText(QApplication::translate("Registration", "Password", nullptr));
        label_7->setText(QApplication::translate("Registration", "REGISTRATION", nullptr));
        pushButton_login->setText(QApplication::translate("Registration", "Log In", nullptr));
        pushButton_registration->setText(QApplication::translate("Registration", "Registrati", nullptr));
        label->setText(QString());
        toolButton->setText(QApplication::translate("Registration", "Rimuovi", nullptr));
        error->setText(QApplication::translate("Registration", "TextLabel", nullptr));
        menuREgistration->setTitle(QApplication::translate("Registration", "Registrazione", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Registration: public Ui_Registration {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTRATION_H
