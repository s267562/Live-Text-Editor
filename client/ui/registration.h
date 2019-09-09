#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QMainWindow>
#include <QObject>
#include "../Networking/Messanger.h"

namespace Ui {
class Registration;
}

class Registration : public QMainWindow
{
    Q_OBJECT

public:
    explicit Registration(QWidget *parent = nullptr);
    void setDefaultProfileIcon();
    void setClient(Messanger *messanger);
    void reset();
    ~Registration();

public slots:
    void on_label_clicked();
    void on_toolButton_clicked();
    void on_pushButton_registration_clicked();
    void on_pushButton_login_clicked();

    void registrationFailed();

signals:
    void showLogin();

private:
    Ui::Registration *ui;
    Messanger *messanger;
    void resizeEvent(QResizeEvent *event);

};

#endif // REGISTRATION_H
