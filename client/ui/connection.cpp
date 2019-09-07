#include "connection.h"
#include "ui_connection.h"

Connection::Connection(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Connection)
{
    ui->setupUi(this);
    this->ui->lineEdit->setText("127.0.0.1");
    connect(this->ui->pushButton, SIGNAL(clicked()), this,SLOT(connectButtonClicked()));

}

Connection::~Connection()
{
    delete ui;
}

void Connection::connectButtonClicked() {
    QString address = this->ui->lineEdit->text();
    emit connectToAddress(address);
}
