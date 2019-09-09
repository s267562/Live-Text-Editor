#include "registration.h"
#include "ui_registration.h"

#include <QFileDialog>
#include <iostream>
#include <QMessageBox>

Registration::Registration(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Registration)
{
    this->ui->setupUi(this);
    ui->error->setVisible(false);
}

Registration::~Registration()
{
    delete ui;
}

void Registration::setClient(Messanger *messanger) {  //TODO: da rimuovere...
    this->messanger = messanger;
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
	QString username = ui->username->text();
	QString password = ui->password->text();
	QPixmap avatar = ui->label->pixmap()->copy(); // TODO controllare se giusto. Togliendo il copy abbiamo un QPixmap*
    QPixmap pix(":/icons/user_icon.jpg");

	if (username == "" || password == ""){
        ui->error->setVisible(true);
	    ui->error->setText("Compile form!");
        return;
	}

	if (messanger != nullptr && username != "" && password != ""){
        ui->error->setVisible(false);
	    messanger->registration(username, password, avatar);
	}
}

void Registration::on_pushButton_login_clicked(){
    this->hide();
    emit this->showLogin();
}

void Registration::registrationFailed(){
    QMessageBox::warning(this,"Registration", "Username and/or password is not correct, try again!");
}

void Registration::reset() {
    ui->username->clear();
    ui->password->clear();
}

QString Registration::getUsername() {
    return ui->username->text();
}

void Registration::resizeEvent(QResizeEvent *event) {
    int centralWidgetX = 0;
    int centralWidgetY = 0;
    int widgetX = 0;
    int widgetY = 0;


    if (width() > ui->centralWidget_2->width()){
        centralWidgetX = width()/2 - ui->centralWidget_2->width()/2;
        widgetX = ui->centralWidget_2->width()/2 - ui->widget->width()/2;
    }else{
        centralWidgetX = 0;
        widgetX = width()/2 - ui->widget->width()/2;
    }

    if (height() > ui->widget->height()){
        widgetY = height()/2 - ui->widget->height()/2;
    } else{
        widgetY = ui->widget->geometry().y();
    }


    ui->centralWidget_2->setGeometry(centralWidgetX, 0, ui->centralWidget_2->width(), height());

    if (widgetX != 0 || widgetY != 0)
        ui->widget->setGeometry(widgetX, widgetY, ui->widget->width(), ui->widget->height());
}