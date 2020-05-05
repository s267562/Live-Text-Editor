#include <QFileDialog>
#include <QBuffer>
#include "editaccount.h"
#include "ui_editaccount.h"

EditAccount::EditAccount(QWidget *parent, User *user) :
    QDialog(parent), user(user),
    ui(new Ui::EditAccount)
{
    ui->setupUi(this);
    ui->username->setText(user->getUsername());
    ui->error->setVisible(false);
    ui->avatar->setPixmap(user->getAvatar());
    connect(ui->editButton, SIGNAL(clicked()), this, SLOT(sendEdit()));
    connect(ui->avatar, SIGNAL(clicked()), this, SLOT(on_label_clicked()));
    connect(    ui->removeButton, SIGNAL(clicked()), this, SLOT(removeImage()));
}

EditAccount::~EditAccount()
{
    delete ui;
}

void EditAccount::on_label_clicked() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose"), "", tr("Images (*.png *.jpeg *.jpg)"));
    if (!filename.isEmpty()) {
        QImage image;
        bool valid = image.load(filename);

        if (valid) {
            QPixmap pix(QPixmap::fromImage(image));
            int w = pix.width();
            int h = pix.height();
            ui->avatar->setPixmap(pix.scaled(w, h, Qt::KeepAspectRatio));
        }
    }
}

void EditAccount::keyPressEvent(QKeyEvent *event) {
    if (event->key() == 16777220)           // Kenter Key is pressed
        sendEdit();
}

void EditAccount::sendEdit(){
    QRegExp rx("^[a-zA-Z0-9]*$");
    if (ui->username->text() == user->getUsername() && ui->newPassword->text() == ""
                    && ui->avatar->pixmap()->toImage() == user->getAvatar().toImage()){
        ui->error->setText("Please, edit your account!");
        ui->error->setVisible(true);
        return;
    }else if (ui->oldPassword->text() == ""){
        ui->error->setText("Please, insert your current password!");
        ui->error->setVisible(true);
        return;
    }else if (rx.indexIn(ui->username->text()) == -1) {
        ui->error->setVisible(true);
        ui->error->setText("Insert valid username");
        return;
    }

    ui->error->setVisible(false);

    QString username;
    QByteArray avatar;
    if (ui->username->text() == user->getUsername()){
        username = "";
    }else{
        username = ui->username->text();
    }

    if (ui->avatar->pixmap()->toImage() != user->getAvatar().toImage()){
        QBuffer buffer(&avatar);
        ui->avatar->pixmap()->toImage().save(&buffer, "PNG");
    } else {
        avatar.clear();
    }

    emit edit(username, ui->newPassword->text(), ui->oldPassword->text(), avatar);
}

void EditAccount::removeImage() {
    QPixmap pix(":/rec/img/user.png");
    ui->avatar->setPixmap(pix);
    ui->avatar->setGeometry(ui->avatar->geometry().x(), ui->avatar->geometry().y(), 80, 80);
}