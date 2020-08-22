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

private:
	QTextCursor textCursor;
	QColor color;
	QLabel cursorLabel;
	QLabel username;
	bool owner;
	bool isSelected = false;
public:
	bool isSelected1() const;

	void setIsSelected(bool isSelected);

public:
	OtherCursor(QString name, QTextDocument *doc, QColor color, bool owner, QWidget *parent = nullptr);


	void setOtherCursorPosition(int position);

	void move(QRect coordinates, int width, int height);


	const QTextCursor &getTextCursor() const;

	const QColor &getColor() const;

	const QLabel &getCursorLabel() const;

	const QLabel &getUsername() const;

	void hide();

};


#endif //TEXTEDITOR_OTHERCURSOR_H
