#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QMainWindow>

namespace Ui {
class Registration;
}

class Registration : public QMainWindow
{
    Q_OBJECT

public:
    explicit Registration(QWidget *parent = nullptr);
    void setDefaultProfileIcon();
    void reset();
    ~Registration();

private slots:

    void on_label_clicked();

    void on_toolButton_clicked();

    void on_pushButton_registration_clicked();

    void on_pushButton_login_clicked();

    signals:
    void showLogin();

private:
    Ui::Registration *ui;
    void resizeEvent(QResizeEvent *event);

};

#endif // REGISTRATION_H
