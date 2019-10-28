//
// Created by Eugenio Marinelli on 2019-10-13.
//

#ifndef TEXTEDITOR_OTHERCURSOR_H
#define TEXTEDITOR_OTHERCURSOR_H


#include <QLabel>
#include <QTextCursor>
#include "Character.h"

class OtherCursor {

public: //TODO: Remove public in the end

    QTextCursor textCursor;
    QColor color;
    Character lastChar;


    OtherCursor(QTextDocument *doc, QColor color, Character character);

    const QTextCursor &getOtherCursor() const;

    void setOtherCursorPosition(int position);

};



#endif //TEXTEDITOR_OTHERCURSOR_H
