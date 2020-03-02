#include "fileinformation.h"
#include "ui_fileinformation.h"
#include <QDebug>

FileInformation::FileInformation(QWidget *parent, QString fileName, QStringList usernameList, Controller *controller, bool isOwner) :
    QDialog(parent),
    ui(new Ui::FileInformation)
{
    ui->setupUi(this);
    ui->lineEdit->setText(fileName);
    if (!isOwner) {
        ui->lineEdit->setEnabled(false);
        ui->save->setEnabled(false);
        ui->cancel->setEnabled(false);
        ui->deleteFile->setEnabled(false);
        this->setMinimumHeight(380);
        this->setMaximumHeight(380);
    }else{
        connect(ui->save, SIGNAL(clicked()), this, SLOT(pushSaveButton()));
        connect(ui->cancel, SIGNAL(clicked()), this, SLOT(pushCancelButton()));
        connect(ui->deleteFile, SIGNAL(clicked()), this, SLOT(pushDeleteFileButton()));
    }

    this->oldFilename = fileName;
    this->controller = controller;
    for (QString username : usernameList){
        qDebug() << username;
        CustomWidgetUsername *customWidgetUsername = new CustomWidgetUsername(this, username, this, isOwner);
        QListWidgetItem *item = new QListWidgetItem(username);
        item->setSizeHint(QSize(0, 40));
        this->ui->usernameList->addItem(item);
        this->ui->usernameList->setItemWidget(item, customWidgetUsername);
    }

}

FileInformation::~FileInformation()
{
    delete ui;
}

void FileInformation::addRemoveUser(QString username){
    if (this->usernames.find(username) == this->usernames.end()) {
        this->usernames.insert(username);
    }else{
        this->usernames.erase(username);
    }
}

void FileInformation::pushSaveButton(){
    qDebug() << oldFilename << ui->lineEdit->text();
    if (!this->usernames.empty() || oldFilename != ui->lineEdit->text()){
        QString owner = controller->getUser()->getUsername();
        QString newFilename = oldFilename != ui->lineEdit->text()? owner + "%_##$$$##_%" + ui->lineEdit->text() : "";
        QStringList usernameList;
        qDebug() << oldFilename << newFilename << ui->lineEdit->text();
        qDebug() << "Utenti selezionati:";
        // manda il messaggio al server
        for (auto itr = usernames.begin(); itr != usernames.end(); ++itr){
            qDebug() << *itr;
            usernameList.push_back(*itr);
        }

        controller->sendFileInformationChanges(owner + "%_##$$$##_%" + oldFilename, newFilename, usernameList);
    }
}

void FileInformation::pushCancelButton(){
    this->close();
}


void FileInformation::pushDeleteFileButton(){
    QString owner = controller->getUser()->getUsername();
    controller->sendDeleteFile(owner + "%_##$$$##_%" + oldFilename);
}