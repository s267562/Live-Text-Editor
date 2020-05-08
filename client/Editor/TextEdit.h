//
// Created by andrea settimo on 04/05/2020.
//

#ifndef TEXTEDITOR_TEXTEDIT_H
#define TEXTEDITOR_TEXTEDIT_H
#include <QTextEdit>
#include <QMimeData>
#include "../Controller/Controller.h"

class Controller;


class TextEdit : public QTextEdit {
Q_OBJECT

public:
    Controller *controller = nullptr;
    int position;
    TextEdit(QWidget * parent, Controller *controller) : QTextEdit(parent), controller(controller) {}
    void insertFromMimeData(const QMimeData * source) override;
};


#endif //TEXTEDITOR_TEXTEDIT_H
