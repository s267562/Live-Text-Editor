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

void AddFile::on_pushButton_clicked()
{
    emit sendShareCode(ui->shareCode->text());
}
