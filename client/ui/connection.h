#ifndef CONNECTION_H
#define CONNECTION_H

#include <QMainWindow>

namespace Ui {
class Connection;
}

class Connection : public QMainWindow
{
    Q_OBJECT

public:
    explicit Connection(QWidget *parent = nullptr);
    ~Connection();

private:
    Ui::Connection *ui;
    QString address;

    void resizeEvent(QResizeEvent *event);

private slots:
    void connectButtonClicked();

signals:
    void connectToAddress(QString address);

};

#endif // CONNECTION_H
