#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include "../../Networking/Messanger.h"
#include "../../Controller.h"
#include "Registration/registration.h"

namespace Ui {
class Login;
}

class Controller;

class Login : public QMainWindow
{
    Q_OBJECT
public:
    explicit Login(QWidget *parent = nullptr, Controller *controller = nullptr);
    void setClient(Messanger *client);
    QString getUsername();
    void reset();
    ~Login();

private:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);

public slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void errorConnection();
    void loginFailed();
   // void loginDone();
   // void onOkButtonClicked();


signals:
    void disconnect();
    void loginSuccessful();
    void showRegistration();

private:
    Ui::Login *ui;
    Messanger* messanger;
    Controller *controller;
};

#endif // MAINWINDOW_H
