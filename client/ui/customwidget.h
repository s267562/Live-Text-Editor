#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QListWidgetItem>

namespace Ui {
class CustomWidget;
}

class CustomWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CustomWidget(QWidget *parent = nullptr, QString filename = "", bool owned = false, QString username="");
    ~CustomWidget();

public slots:
    void pushSharedButton();

private:
    Ui::CustomWidget *ui;
    QString filename;
    QString fileShareCode;
};

#endif // CUSTOMWIDGET_H
