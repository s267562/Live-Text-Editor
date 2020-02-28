#ifndef CUSTOMWIDGETUSERNAME_H
#define CUSTOMWIDGETUSERNAME_H

#include <QWidget>
#include <FileInformation/fileinformation.h>
#include <QListWidgetItem>

namespace Ui {
class CustomWidgetUsername;
}

class FileInformation;
class CustomWidgetUsername : public QWidget
{
    Q_OBJECT

public:
    explicit CustomWidgetUsername(QWidget *parent = nullptr, QString username = "", FileInformation *fileIformation = nullptr, bool isOwner = false);
    ~CustomWidgetUsername();

public slots:
    void pushRemoveButton();

private:
    Ui::CustomWidgetUsername *ui;
    FileInformation *fileInformation;
    bool pressed = false;
};

#endif // CUSTOMWIDGETUSERNAME_H
