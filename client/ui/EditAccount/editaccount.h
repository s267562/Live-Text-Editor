#ifndef EDITACCOUNT_H
#define EDITACCOUNT_H

#include <QDialog>
#include "../../User.h"

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

signals:
    void edit(QString username, QString newPassword, QString oldPassword, QByteArray avatar);

private:
    Ui::EditAccount *ui;
    User *user;
};

#endif // EDITACCOUNT_H
