/********************************************************************************
** Form generated from reading UI file 'showFiles.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHOWFILES_H
#define UI_SHOWFILES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ShowFiles
{
public:
    QWidget *centralWidget;
    QListWidget *listWidget;
    QLineEdit *filename;
    QPushButton *pushButton_newFile;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ShowFiles)
    {
        if (ShowFiles->objectName().isEmpty())
            ShowFiles->setObjectName(QString::fromUtf8("ShowFiles"));
        ShowFiles->resize(400, 300);
        centralWidget = new QWidget(ShowFiles);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        listWidget = new QListWidget(centralWidget);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        listWidget->setGeometry(QRect(10, 40, 381, 161));
        filename = new QLineEdit(centralWidget);
        filename->setObjectName(QString::fromUtf8("filename"));
        filename->setGeometry(QRect(10, 10, 261, 21));
        pushButton_newFile = new QPushButton(centralWidget);
        pushButton_newFile->setObjectName(QString::fromUtf8("pushButton_newFile"));
        pushButton_newFile->setGeometry(QRect(276, 6, 113, 32));
        ShowFiles->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ShowFiles);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 22));
        ShowFiles->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ShowFiles);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        ShowFiles->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ShowFiles);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        ShowFiles->setStatusBar(statusBar);

        retranslateUi(ShowFiles);

        QMetaObject::connectSlotsByName(ShowFiles);
    } // setupUi

    void retranslateUi(QMainWindow *ShowFiles)
    {
        ShowFiles->setWindowTitle(QApplication::translate("ShowFiles", "Your Files", nullptr));
#ifndef QT_NO_STATUSTIP
        filename->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        filename->setWhatsThis(QApplication::translate("ShowFiles", "<html><head/><body><p>File name</p><p><br/></p></body></html>", nullptr));
#endif // QT_NO_WHATSTHIS
#ifndef QT_NO_ACCESSIBILITY
        filename->setAccessibleDescription(QString());
#endif // QT_NO_ACCESSIBILITY
        filename->setInputMask(QString());
        filename->setPlaceholderText(QApplication::translate("ShowFiles", " File name", nullptr));
        pushButton_newFile->setText(QApplication::translate("ShowFiles", "New File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ShowFiles: public Ui_ShowFiles {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHOWFILES_H
