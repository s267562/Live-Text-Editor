#include "showFiles.h"
#include "ui_showFiles.h"

ShowFiles::ShowFiles(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShowFiles)
{
    ui->setupUi(this);
//    QStringList l;
//    l.append("A");
//    l.append("B");
//    l.append("C");
//    l.append("D");
//    l.append("E");
//    l.append("E");
//    l.append("E");
//    l.append("E");
//    l.append("E");
//    this->ui->listWidget->addItems(l);
}

ShowFiles::~ShowFiles()
{
    delete ui;
}

void ShowFiles::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{

}

void ShowFiles::on_pushButton_newFile_clicked()
{

}
