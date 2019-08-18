#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include "../Networking/Client.h"

namespace Ui {
class Login;
}

class Login : public QMainWindow
{
    Q_OBJECT
public:
    explicit Login(QWidget *parent = nullptr);
    void setClient(Client *client);
    ~Login();

private slots:
    void on_pushButton_clicked();
    void errorConnection();
    void loginFailed();
    //void loginDone();

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void disconnect();

private:
    Ui::Login *ui;
    Client* client;
};

#endif // MAINWINDOW_H
