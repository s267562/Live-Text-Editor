#include "customwidgetusername.h"
#include "ui_customwidgetusername.h"

CustomWidgetUsername::CustomWidgetUsername(QWidget *parent, QString username, FileInformation *fileInformation, bool isOwner) :
    QWidget(parent),
    ui(new Ui::CustomWidgetUsername)
{
    ui->setupUi(this);
    if (isOwner){
        ui->remove->setPixmap(QPixmap(":/rec/img/substract.png"));
    }else{
        ui->username->setEnabled(false);
        ui->remove->setEnabled(false);
    }
    ui->username->setText(username);
    this->fileInformation = fileInformation;
    connect(ui->remove, SIGNAL(clicked()), this, SLOT(pushRemoveButton()));
}

CustomWidgetUsername::~CustomWidgetUsername()
{
    delete ui;
}

void CustomWidgetUsername::pushRemoveButton(){
    if (fileInformation != nullptr)
        this->fileInformation->addRemoveUser(ui->username->text());

    if (pressed){
        ui->remove->setPixmap(QPixmap(":/rec/img/substract.png"));
        pressed = false;
    }else{
        ui->remove->setPixmap(QPixmap(":/rec/img/add.png"));
        pressed = true;
    }
}
