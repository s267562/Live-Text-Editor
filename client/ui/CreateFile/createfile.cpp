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

void CreateFile::on_pushButton_clicked()
{
    emit createFile(ui->fileName->text());
}
