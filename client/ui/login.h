#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include "../Networking/Messanger.h"
#include "registration.h"

namespace Ui {
class Login;
}

class Login : public QMainWindow
{
    Q_OBJECT
public:
    explicit Login(QWidget *parent = nullptr);
    void setClient(Messanger *client);
    QString getUsername();
    void reset();
    ~Login();

public slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void errorConnection();
    void loginFailed();
   // void loginDone();
   // void onOkButtonClicked();

protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:
    void disconnect();
    void loginSuccessful();
    void showRegistration();

private:
    Ui::Login *ui;
    Messanger* messanger;
};

#endif // MAINWINDOW_H
