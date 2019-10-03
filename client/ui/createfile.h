#ifndef CREATEFILE_H
#define CREATEFILE_H

#include <QDialog>

namespace Ui {
class CreateFile;
}

class CreateFile : public QDialog
{
    Q_OBJECT

public:
    explicit CreateFile(QWidget *parent = nullptr);
    ~CreateFile();

private slots:
    void on_pushButton_clicked();

signals:
    void createFile(QString filename);

private:
    Ui::CreateFile *ui;
};

#endif // CREATEFILE_H
