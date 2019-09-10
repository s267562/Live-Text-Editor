#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
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

    ui->newFile->setPixmap(QPixmap(":/rec/img/new-file.png"));
    ui->logout->setPixmap(QPixmap(":/rec/img/logout.png"));
    ui->avatar->setPixmap(QPixmap(":/rec/img/user.png"));
    connect(ui->newFile, SIGNAL(clicked()), this, SLOT(on_actionNew_File_triggered()));
    connect(ui->logout, SIGNAL(clicked()), this, SLOT(on_actionLogout_triggered()));

    QGraphicsDropShadowEffect *m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setColor(QColor(0, 0, 0, 255 * 0.1));
    m_shadowEffect->setXOffset(0);
    m_shadowEffect->setYOffset(4);
    m_shadowEffect->setBlurRadius(12);
// hide shadow
    m_shadowEffect->setEnabled(true);
    ui->customToolbar->setGraphicsEffect(m_shadowEffect);
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
        ui->listWidget->move(10,102);
        newFileShown = true;
    }else{
        ui->filename->close();
        ui->pushButton_newFile->close();
        ui->listWidget->move(10,70);
        newFileShown = false;
    }

}

void ShowFiles::on_actionLogout_triggered(){
    QMessageBox::information(this, "Logout", "Logout!");
    emit logout();
}

void ShowFiles::resizeEvent(QResizeEvent *event) {
    ui->customToolbar->setGeometry(0, 0, width(), 60);
}