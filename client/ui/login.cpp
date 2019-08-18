#include "login.h"
#include "ui_login.h"
#include <QMessageBox>

Login::Login(QWidget *parent): QMainWindow(parent), ui(new Ui::Login){
    ui->setupUi(this);

}

Login::~Login(){
    delete ui;
}

void Login::setClient(Client *client) {
    this->client = client;
    connect(this->client, &Client::errorConnection, this, &Login::errorConnection);
    connect(this->client, &Client::loginFailed, this, &Login::loginFailed);
    //connect(this, &Login::disconnect, this->client, &Client::onDisconnect);
}

void Login::on_pushButton_clicked()
{
    QString username = ui->username->text();
    QString password = ui->password->text();

    /*if(username ==  "test" && password == "test") {
        QMessageBox::information(this, "Login", "Username and password is correct");
    }else {
        QMessageBox::warning(this,"Login", "Username and password is not correct");
    }*/

    client->logIn(username,password);
}

void Login::errorConnection(){
    QMessageBox::information(this, "Connection", "Try again, connection not established!");
}

void Login::loginFailed(){
    QMessageBox::warning(this,"Login", "Username and/or password is not correct, try again!");
}

void Login::closeEvent(QCloseEvent *event){
    //emit disconnect();
    //client->logOut();
}