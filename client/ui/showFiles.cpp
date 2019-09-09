#include <QMessageBox>
#include "showFiles.h"
#include "ui_showFiles.h"
#include "../Networking/Messanger.h"

ShowFiles::ShowFiles(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShowFiles)
{
    ui->setupUi(this);
    //connect(this,SIGNAL(newFile(QString)),this->parent(), SLOT(requestForFile(QString)));
    ui->filename->close();
    ui->pushButton_newFile->close();
}

ShowFiles::~ShowFiles()
{
    delete ui;
}

void ShowFiles::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString filename = item->text();

    emit newFile(filename);
}

void ShowFiles::addFiles(QStringList l){
    this->ui->listWidget->clear();
    this->ui->listWidget->addItems(l);
}

void ShowFiles::on_pushButton_newFile_clicked()
{
    QString filename = this->ui->filename->text();
    emit newFile(filename);
}

void ShowFiles::on_actionNew_File_triggered(){
    //QMessageBox::information(this, "File", "File!");
    if (!newFileShown){
        ui->filename->show();
        ui->pushButton_newFile->show();
        ui->listWidget->move(10,50);
        newFileShown = true;
    }else{
        ui->filename->close();
        ui->pushButton_newFile->close();
        ui->listWidget->move(10,10);
        newFileShown = false;
    }

}

void ShowFiles::on_actionLogout_triggered(){
    QMessageBox::information(this, "Logout", "Logout!");
    emit logout();
}