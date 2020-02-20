#include "createfile.h"
#include "ui_createfile.h"

CreateFile::CreateFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateFile)
{
    ui->setupUi(this);
}

CreateFile::~CreateFile()
{
    delete ui;
}

void CreateFile::keyPressEvent(QKeyEvent *event) {
    if (event->key() == 16777220)           // Kenter Key is pressed
        on_pushButton_clicked();
}

void CreateFile::on_pushButton_clicked()
{
    if (ui->fileName->text() != "")
        emit createFile(ui->fileName->text());
}
