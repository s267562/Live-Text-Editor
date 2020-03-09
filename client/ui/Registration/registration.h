#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QMainWindow>
#include <QObject>
#include "../../Networking/Messanger.h"
#include "../../Controller/Controller.h"

namespace Ui {
class Registration;
}

class Controller;
class Messanger;

class Registration : public QMainWindow
{
    Q_OBJECT

public:
    explicit Registration(QWidget *parent = nullptr, Controller *controller = nullptr);
    void setDefaultProfileIcon();
    void setClient(Messanger *messanger);
    QString getUsername();
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
    Controller *controller;

    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // REGISTRATION_H
