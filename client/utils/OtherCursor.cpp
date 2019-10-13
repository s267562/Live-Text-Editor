//
// Created by Eugenio Marinelli on 2019-10-13.
//

#include "OtherCursor.h"


OtherCursor::OtherCursor(QString userName, QWidget *parent): QLabel(userName,parent) {

    textCursor.setPosition(0);

}

const QTextCursor &OtherCursor::getOtherCursor() const {
    return textCursor;
}

void OtherCursor::setOtherCursor(const QTextCursor &textCursor) {
    OtherCursor::textCursor = textCursor;
}

OtherCursor::~OtherCursor() {

}

