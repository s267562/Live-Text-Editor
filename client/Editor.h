//
// Created by simone on 05/08/19.
//

#ifndef TEXTEDITOR_EDITOR_H
#define TEXTEDITOR_EDITOR_H

#include "Controller.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>
#include <QApplication>
#include <QTextEdit>
#include <iostream>

class Controller;

class Editor : public QMainWindow {
public:
    Editor(QString siteId, QWidget *parent = nullptr);

    void setController(Controller *controller);
    void insertChar(char character, Pos pos);
    void deleteChar(Pos pos);

private slots:
    void onTextChanged(int position, int charsRemoved, int charsAdded);

private:
    QTextEdit *textEdit;
    QString siteId;
    QTextCursor cursor;
    Controller *controller;

};

#endif //TEXTEDITOR_EDITOR_H
