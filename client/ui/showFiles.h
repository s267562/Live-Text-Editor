#ifndef SHOWFILES_H
#define SHOWFILES_H

#include <QListWidget>
#include <QMainWindow>
#include "../Controller.h"

namespace Ui {
class ShowFiles;
}

class Controller;

class ShowFiles : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShowFiles(QWidget *parent = nullptr, Controller *controller = nullptr);
    ~ShowFiles();
    void addFiles(std::map<QString, bool> l);
    void addFile(std::map<QString, bool> l);

public slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_pushButton_newFile_clicked();
    void on_actionNew_File_triggered();
    void on_actionLogout_triggered();
    void on_actionAdd_File_triggered();
    void showError();
    void editAccount();

signals:
    void newFile(QString filename);
    void logout();

private:
    Ui::ShowFiles *ui;
    bool newFileShown = false;
    Controller *controller;

    void resizeEvent(QResizeEvent *event);

    static QString getShareCode(const QString& username,const QString& filename);
};

#endif // SHOWFILES_H
