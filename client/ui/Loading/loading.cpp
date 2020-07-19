#include "Loading/loading.h"
#include "ui_loading.h"
#include <QMovie>

Loading::Loading(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Loading)
{
    ui->setupUi(this);
}

Loading::~Loading()
{
    delete ui;
}
