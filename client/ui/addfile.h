#ifndef ADDFILE_H
#define ADDFILE_H

#include <QDialog>

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
};

#endif // ADDFILE_H
