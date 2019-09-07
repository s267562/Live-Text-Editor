#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <iostream>

Login::Login(QWidget *parent): QMainWindow(parent), ui(new Ui::Login) {
    ui->setupUi(this);
    ui->label_3->setVisible(false);
    //connect(this, SIGNAL(showRegistration()), this->parent(), SLOT(showRegistration()));
    //connect(this,SIGNAL(loginSuccessful()),this->parent(), SLOT(showFileFinder()));
}

Login::~Login(){
    delete ui;
}

void Login::setClient(Messanger *messanger) {  //TODO: da rimuovere...
    this->messanger = messanger;

    connect(this->messanger, &Messanger::loginFailed, this, &Login::loginFailed);
}

void Login::on_pushButton_clicked()
{
    QString username = ui->username->text();
    QString password = ui->password->text();

    if (ui->username->text() == "" && ui->password->text() == ""){
        ui->label_3->setText("Insert username and password");
        ui->label_3->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        ui->label_3->setVisible(true);
    }else if (ui->username->text() == ""){
        ui->label_3->setText("Insert username");
        ui->label_3->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        ui->label_3->setVisible(true);
    }else if (ui->password->text() == "") {
        ui->label_3->setText("Insert password");
        ui->label_3->setStyleSheet(QStringLiteral("QLabel{color: red;}"));
        ui->label_3->setVisible(true);
    }else{

        bool result = messanger->logIn(username,password);

        ui->label_3->setVisible(false);
        if (result) {
            emit loginSuccessful();
        }
    }
}

void Login::errorConnection(){
    QMessageBox::information(this, "Connection", "Try again, connection not established!");
}

void Login::loginFailed(){
    QMessageBox::warning(this,"Login", "Username and/or password is not correct, try again!");
}

void Login::closeEvent(QCloseEvent *event){
    emit disconnect();
}

void Login::on_pushButton_2_clicked() {
    emit showRegistration();
}

QString Login::getUsername(){
    return ui->username->text();
}

void Login::reset(){
    ui->username->clear();
    ui->password->clear();
}

/**
void Login::onOkButtonClicked()
{
    QString username = ui->username->text();
    QString password = ui->password->text();

    if (username == "test" &&
        password == "test") {
        emit loginSuccessful();
    } else {
            QMessageBox::warning(this, tr("Error"), tr("Invalid username/password combination"));
        ui->username->clear();
        ui->password->clear();
    }
}


void Login::loginDone(){
    //this->messanger->requestForFile("CiaoTy!");
    emit loginSuccessful();
}
 */

void Login::resizeEvent(QResizeEvent *event) {
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

    /*if (height() > ui->centralWidget_2->height()){

    }*/

    /*if (width()/2 - ui->centralWidget_2->width()/2 > 0){
        centralWidgetX = width()/2 - ui->centralWidget_2->width()/2;
    }else{
        //widgetX = ui->centralWidget_2->width()/2 - ui->widget->width()/2;
    }*/

    ui->centralWidget_2->setGeometry(centralWidgetX, 0, ui->centralWidget_2->width(), height());

    if (height()/2 - ui->widget->height() > 0){
        widgetY = height()/2 - ui->widget->height();

    }

    /*if (ui->centralWidget_2->width()/2 - ui->widget->width()/2 > 0 && width()/2 - ui->centralWidget_2->width()/2 > 0){
        widgetX = ui->centralWidget_2->width()/2 - ui->widget->width()/2;
    }*/

    if (widgetX != 0 || widgetY != 0)
        ui->widget->setGeometry(widgetX, widgetY, ui->widget->width(), ui->widget->height());

}