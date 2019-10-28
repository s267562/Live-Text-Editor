//
// Created by Eugenio Marinelli on 2019-10-13.
//

#include "OtherCursor.h"


OtherCursor::OtherCursor(QTextDocument *doc, QColor color, Character character): textCursor(doc), color(color), lastChar(character) {

    textCursor.setPosition(0);

}

const QTextCursor &OtherCursor::getOtherCursor() const {
    return textCursor;
}

void OtherCursor::setOtherCursorPosition(int position) {
    textCursor.setPosition(position);
}
