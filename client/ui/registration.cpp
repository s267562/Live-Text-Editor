#include "registration.h"
#include "ui_registration.h"

#include <QFileDialog>
#include <iostream>

Registration::Registration(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Registration)
{
    this->ui->setupUi(this);

}

Registration::~Registration()
{
    delete ui;
}


void Registration::on_label_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose"),"",tr("Images (*.png *.jpeg *.jpg)"));
    if(QString::compare(filename,QString())!=0){
        QImage image;
        bool valid = image.load(filename);

        if(valid){

            QPixmap pix(QPixmap::fromImage(image));
            int w=ui->label->width();
            int h=ui->label->height();
            ui->label->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
        }
    }
}

void Registration::on_toolButton_clicked()
{
    setDefaultProfileIcon();
}

void Registration::setDefaultProfileIcon(){
    QPixmap pix(":/icons/user_icon.jpg");
    int w=ui->label->width();
    int h=ui->label->height();
    ui->label->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
}


void Registration::on_pushButton_registration_clicked(){

}

void Registration::on_pushButton_login_clicked(){
    this->hide();
    emit this->showLogin();
}

void Registration::reset() {
    ui->username->clear();
    ui->password->clear();
}