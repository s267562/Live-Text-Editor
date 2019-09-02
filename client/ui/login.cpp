#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <iostream>

Login::Login(QWidget *parent): QMainWindow(parent), ui(new Ui::Login) {
    ui->setupUi(this);
    ui->label_3->setVisible(false);
    connect(this, SIGNAL(showRegistration()), this->parent(), SLOT(showRegistration()));
    connect(this,SIGNAL(loginSuccessful()),this->parent(), SLOT(showFileFinder()));
}

Login::~Login(){
    delete ui;
}

void Login::setClient(Client *client) {
    this->client = client;
    //connect(this->client, &Client::errorConnection, this, &Login::errorConnection);
    connect(this->client, &Client::loginFailed, this, &Login::loginFailed);
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

        bool result=client->logIn(username,password);

        ui->label_3->setVisible(false);
        if( result ) {
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
    //this->client->requestForFile("CiaoTy!");
    emit loginSuccessful();
}
 */