#include "login.h"
#include "ui_login.h"
#include <QMessageBox>

Login::Login(QWidget *parent): QMainWindow(parent), ui(new Ui::Login){
    ui->setupUi(this);
    ui->label_3->setVisible(false);
}

Login::~Login(){
    delete ui;
}

void Login::setClient(Client *client) {
    this->client = client;
    connect(this->client, &Client::errorConnection, this, &Login::errorConnection);
    connect(this->client, &Client::loginFailed, this, &Login::loginFailed);
    connect(this, &Login::disconnect, this->client, &Client::onDisconnect);
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
        client->logIn(username,password);
        ui->label_3->setVisible(false);
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
    //client->logOut();
}