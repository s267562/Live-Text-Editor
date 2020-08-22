#include "addfile.h"
#include "ui_addfile.h"

AddFile::AddFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFile)
{
    ui->setupUi(this);
}

AddFile::~AddFile()
{
    delete ui;
}

void AddFile::keyPressEvent(QKeyEvent *event) {
    if (event->key() == 16777220)           // Kenter Key is pressed
        on_pushButton_clicked();
}
void AddFile::on_pushButton_clicked()
{
    if (ui->shareCode->text() != "")
        emit sendShareCode(ui->shareCode->text());
}
