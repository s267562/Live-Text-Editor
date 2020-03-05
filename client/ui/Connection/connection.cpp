#include "connection.h"
#include "ui_connection.h"
#include <QDebug>
Connection::Connection(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Connection)
{
    ui->setupUi(this);
    this->ui->serverIP->setText("192.168.1.174");
    this->ui->serverPort->setText("1234");
    connect(this->ui->pushButton, SIGNAL(clicked()), this,SLOT(connectButtonClicked()));

}

Connection::~Connection()
{
    delete ui;
}

void Connection::keyPressEvent(QKeyEvent *event) {
    if (event->key() == 16777220)           // Kenter Key is pressed
        connectButtonClicked();
}

void Connection::connectButtonClicked() {
    QString address = this->ui->serverIP->text();
    QString port = this->ui->serverPort->text();
    emit connectToAddress(address, port);
}

void Connection::resizeEvent(QResizeEvent *event) {
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
