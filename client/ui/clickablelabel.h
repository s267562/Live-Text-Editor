#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <Qt>
#include <QGraphicsDropShadowEffect>

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ClickableLabel();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);

protected:
    void enterEvent(QEvent *ev){
        int height = geometry().height();
        int width = geometry().width();
        resize(width + 1, height + 1);
    }

    void leaveEvent(QEvent *ev){
        int height = geometry().height();
        int width = geometry().width();
        resize(width - 1, height - 1);
    }

};

#endif // CLICKABLELABEL_H
