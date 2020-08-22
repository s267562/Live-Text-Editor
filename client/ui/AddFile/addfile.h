#ifndef ADDFILE_H
#define ADDFILE_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class AddFile;
}

class AddFile : public QDialog
{
    Q_OBJECT

public:
    explicit AddFile(QWidget *parent = nullptr);
    ~AddFile();

private slots:
    void on_pushButton_clicked();

signals:
    void sendShareCode(QString);

private:
    Ui::AddFile *ui;
    void keyPressEvent(QKeyEvent *event);
};

#endif // ADDFILE_H
