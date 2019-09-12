#include "sharefile.h"
#include "ui_sharefile.h"

ShareFile::ShareFile(QWidget *parent, QString filename) :
    QDialog(parent), filename(filename),
    ui(new Ui::ShareFile)
{
    ui->setupUi(this);
    ui->title->setText("Share " + filename);
    ui->copyIcon->setPixmap(QPixmap(":/rec/img/copy.png"));
}

ShareFile::~ShareFile()
{
    delete ui;
}
