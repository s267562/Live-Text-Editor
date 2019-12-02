//
// Created by Eugenio Marinelli on 2019-10-13.
//

#include "OtherCursor.h"


OtherCursor::OtherCursor(QString name, QTextDocument *doc, QColor color, QWidget *parent): QObject(parent), username(name, parent), cursorLabel(parent), textCursor(doc), color(color) {

    textCursor.setPosition(0);

}

const QTextCursor &OtherCursor::getOtherCursor() const {
    return textCursor;
}

void OtherCursor::setOtherCursorPosition(int position) {
    textCursor.setPosition(position);
}

void OtherCursor::move(QRect coordinates, int width, int height) {

    this->cursorLabel.show();
    this->username.show();
    this->cursorLabel.move(coordinates.topRight());
    this->cursorLabel.setFixedHeight(height);
    this->cursorLabel.setFixedWidth(width);
    this->cursorLabel.setStyleSheet("color: "+color.name()+"; background:"+this->color.name()+";");
    this->username.move(coordinates.right(),coordinates.top()-5);
    this->username.setStyleSheet("background-color: "+this->color.name(QColor::HexArgb));

}
