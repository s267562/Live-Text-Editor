#include "showFiles.h"
#include "ui_showFiles.h"

ShowFiles::ShowFiles(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShowFiles)
{
    ui->setupUi(this);
    connect(this,SIGNAL(newFile(QString)),this->parent(), SLOT(requestForFile(QString)));
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
    QString filename = item->text();

    emit newFile(filename);
}

void ShowFiles::addFiles(QStringList l){
    this->ui->listWidget->addItems(l);
}

void ShowFiles::on_pushButton_newFile_clicked()
{
    QString filename = this->ui->filename->text();
    emit newFile(filename);
}
