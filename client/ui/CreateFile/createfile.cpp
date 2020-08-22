#include <QMessageBox>
#include "createfile.h"
#include "ui_createfile.h"
#include <QDebug>

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
    QRegExp rx("^[A-Za-z0-9 ]*[A-Za-z0-9][A-Za-z0-9 ]*$");
    if (ui->fileName->text() != "" && rx.indexIn(ui->fileName->text()) != -1)
        emit createFile(ui->fileName->text());
    else {
        QMessageBox::warning(this, "Error filename", "Insert a valid filename!");
    }
}
