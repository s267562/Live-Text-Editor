//
// Created by Eugenio Marinelli on 2019-10-13.
//

#ifndef TEXTEDITOR_OTHERCURSOR_H
#define TEXTEDITOR_OTHERCURSOR_H


#include <QLabel>
#include <QTextCursor>

class OtherCursor : public QLabel{
Q_OBJECT

    QTextCursor textCursor;

public:
    OtherCursor(QString userName, QWidget *parent);

    const QTextCursor &getOtherCursor() const;

    void setOtherCursor(const QTextCursor &textCursor);

    virtual ~OtherCursor();
};



#endif //TEXTEDITOR_OTHERCURSOR_H
