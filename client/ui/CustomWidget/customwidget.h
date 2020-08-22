#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QListWidgetItem>
#include "../../Controller/Controller.h"

namespace Ui {
class CustomWidget;
}

class Controller;

class CustomWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomWidget(QWidget *parent = nullptr, QString filename = "", QString owner = "", bool owned = false, QString shareCode="ERROR", Controller *controller = nullptr);
    ~CustomWidget();
    void createFileInformation(QStringList usernameList);

public slots:
    void pushSharedButton();
    void pushSettingsButton();

private:
    Ui::CustomWidget *ui;
    QString filename;
    QString fileShareCode;
    Controller *controller;
    QString owner;
};

#endif // CUSTOMWIDGET_H
