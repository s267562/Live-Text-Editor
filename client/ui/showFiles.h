#ifndef SHOWFILES_H
#define SHOWFILES_H

#include <QListWidget>
#include <QMainWindow>

namespace Ui {
class ShowFiles;
}

class ShowFiles : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShowFiles(QWidget *parent = nullptr);
    ~ShowFiles();
    void addFiles(QStringList l);

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_newFile_clicked();

    signals:
    void newFile(QString filename);

private:
    Ui::ShowFiles *ui;
};

#endif // SHOWFILES_H
