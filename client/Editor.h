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
    Editor(std::string siteId, QWidget *parent = nullptr);
    void setController(Controller *controller);
    const std::string &getSiteId() const;
    QTextEdit *getTextEdit() const;

private slots:
    void onTextChanged(int position, int charsRemoved, int charsAdded);

private:
    QTextEdit *textEdit;
    std::string siteId;
    QTextCursor cursor;
    Controller *controller;
};

#endif //TEXTEDITOR_EDITOR_H
