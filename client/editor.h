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

namespace Ui {
    class Editor;
}

class Editor : public QMainWindow {
    Q_OBJECT
public:
    Editor(QString siteId, QWidget *parent = nullptr);

    void setController(Controller *controller);
    void insertChar(char character, Pos pos);
    void deleteChar(Pos pos);
    ~Editor();

public slots:
    void onTextChanged(int position, int charsRemoved, int charsAdded);
    void on_actionNew_File_triggered();
    void on_actionShare_file_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_as_PDF_triggered();
    void on_actionLogout_triggered();
    void removeUser(QString user);
    void setUsers(QStringList user);

signals:
    void logout();
    void showFinder();

private:
    Ui::Editor *ui;
    QTextEdit *textEdit;
    QTextDocument *textDocument;
    QTextCursor textCursor;
    int cursorPos;
    int startSelection;
    int endSelection;
    QString siteId;
    Controller *controller;
    QStringList users;

    void undo();
    void redo();
    bool validSignal(int i, int i1, int i2);
    void resizeEvent (QResizeEvent *event);
    void restoreCursor();
    void saveCursor();
};

#endif //TEXTEDITOR_EDITOR_H
