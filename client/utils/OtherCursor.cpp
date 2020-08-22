//
// Created by Eugenio Marinelli on 2019-10-13.
//

#include "OtherCursor.h"


OtherCursor::OtherCursor(QString name, QTextDocument *doc, QColor color, bool owner, QWidget *parent) : QObject(parent),
																										username(name,
																												 parent),
																										cursorLabel(
																												parent),
																										textCursor(doc),
																										color(color),
																										owner(owner) {

	textCursor.setPosition(0);
	if (!owner)
		cursorLabel.show();

}


void OtherCursor::setOtherCursorPosition(int position) {
	textCursor.setPosition(position);
}

void OtherCursor::move(QRect coordinates, int width, int height) {
	if (!owner) {
		this->cursorLabel.show();
		this->username.show();
		this->cursorLabel.move(coordinates.topRight());
		this->cursorLabel.setFixedHeight(height);
		this->cursorLabel.setFixedWidth(width);
		this->cursorLabel.setStyleSheet("color: " + color.name() + "; background:" + this->color.name() + ";");
		this->username.move(coordinates.right(), coordinates.top() - 5);
		this->username.setStyleSheet("background-color: " + this->color.name(QColor::HexArgb));
	}
}

const QTextCursor &OtherCursor::getTextCursor() const {
	return textCursor;
}

const QColor &OtherCursor::getColor() const {
	return color;
}

const QLabel &OtherCursor::getCursorLabel() const {
	return cursorLabel;
}

const QLabel &OtherCursor::getUsername() const {
	return username;
}

void OtherCursor::hide() {
	this->cursorLabel.hide();
	this->username.hide();
}

bool OtherCursor::isSelected1() const {
	return isSelected;
}

void OtherCursor::setIsSelected(bool isSelected) {
	OtherCursor::isSelected = isSelected;
}

