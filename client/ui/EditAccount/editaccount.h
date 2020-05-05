#ifndef EDITACCOUNT_H
#define EDITACCOUNT_H

#include <QDialog>
#include "../../User/User.h"
#include <QKeyEvent>

namespace Ui {
class EditAccount;
}

class EditAccount : public QDialog
{
    Q_OBJECT

public:
    explicit EditAccount(QWidget *parent = nullptr, User *user = nullptr);
    ~EditAccount();

public slots:
    void sendEdit();
    void on_label_clicked();
    void removeImage();

signals:
    void edit(QString username, QString newPassword, QString oldPassword, QByteArray avatar);

private:
    Ui::EditAccount *ui;
    User *user;
    void keyPressEvent(QKeyEvent *event);
};

#endif // EDITACCOUNT_H
