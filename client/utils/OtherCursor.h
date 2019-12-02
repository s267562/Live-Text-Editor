//
// Created by Eugenio Marinelli on 2019-10-13.
//

#ifndef TEXTEDITOR_OTHERCURSOR_H
#define TEXTEDITOR_OTHERCURSOR_H


#include <QLabel>
#include <QTextCursor>
#include "Character.h"

class OtherCursor : public QObject {
Q_OBJECT

public: //TODO: Remove public in the end

    QTextCursor textCursor;
    QColor color;
    QLabel cursorLabel;
    QLabel username;


    OtherCursor(QString name, QTextDocument *doc, QColor color, QWidget *parent=nullptr);

    const QTextCursor &getOtherCursor() const;

    void setOtherCursorPosition(int position);

    void move(QRect coordinates, int width, int height);

};



#endif //TEXTEDITOR_OTHERCURSOR_H
