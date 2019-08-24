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
    ~Registration();
    void setDefaultProfileIcon();

private slots:

    void on_label_clicked();

    void on_toolButton_clicked();

private:
    Ui::Registration *ui;

};

#endif // REGISTRATION_H
