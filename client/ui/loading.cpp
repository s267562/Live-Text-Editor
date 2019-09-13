#include "loading.h"
#include "ui_loading.h"
#include <QMovie>

Loading::Loading(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Loading)
{
    ui->setupUi(this);
    QMovie *movie = new QMovie(":/rec/img/loading.gif");
    ui->loadingGIF->setMovie(movie);
    movie->start();
}

Loading::~Loading()
{
    delete ui;
}
