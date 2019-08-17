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
    this->client = std::shared_ptr<Client>(client);
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
