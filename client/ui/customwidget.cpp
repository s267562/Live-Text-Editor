#include <QMessageBox>
#include "customwidget.h"
#include "ui_customwidget.h"

CustomWidget::CustomWidget(QWidget *parent, QString filename, bool isShared) :
        QWidget(parent),
    ui(new Ui::CustomWidget)
{
    ui->setupUi(this);
    ui->share->setPixmap(QPixmap(":/rec/img/share.png"));
    ui->fileName->setText(filename);
    if (isShared){
        ui->network->setPixmap(QPixmap(":/rec/img/network.png"));
    }

    connect(ui->share, SIGNAL(clicked()), this, SLOT(pushSharedButton()));
}

CustomWidget::~CustomWidget()
{
    delete ui;
}

void CustomWidget::pushSharedButton(){
    QMessageBox::information(this, "Share", "Share");
}
