//
// Created by simone on 05/08/19.
//

#ifndef TEXTEDITOR_EDITOR_H
#define TEXTEDITOR_EDITOR_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>
#include "CRDT.h"

class Editor : public QMainWindow {
public:
    Editor(std::string siteId, CRDT crdt, QWidget *parent = nullptr);
    const std::string &getSiteId() const;

private slots:
    void onTextChanged(int position, int charsRemoved, int charsAdded);

private:
    QTextEdit *textEdit;
    std::string siteId;
    CRDT crdt;
    QTextCursor cursor;
};

#endif //TEXTEDITOR_EDITOR_H
