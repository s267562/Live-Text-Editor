#ifndef FILEINFORMATION_H
#define FILEINFORMATION_H

#include <QDialog>
#include "CustomWidgetUsername/customwidgetusername.h"
#include "../../Controller.h"
#include <set>

namespace Ui {
class FileInformation;
}

class CustomWidgetUsername;

class FileInformation : public QDialog
{
    Q_OBJECT

public:
    explicit FileInformation(QWidget *parent = nullptr, QString fileName = "", QStringList usernameList = {}, Controller *controller = nullptr, bool isOwner = false);
    ~FileInformation();
    void addRemoveUser(QString username);

public slots:
    void pushSaveButton();
    void pushCancelButton();
    void pushDeleteFileButton();


private:
    Ui::FileInformation *ui;
    std::set <QString, std::greater <QString>> usernames;
    QString oldFilename;
    Controller *controller;
};

#endif // FILEINFORMATION_H
