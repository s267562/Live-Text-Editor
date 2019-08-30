//
// Created by simone on 05/08/19.
//

#include "editor.h"
#include "ui_editor.h"
#include <QMenuBar>
#include <QMenu>
#include <QSlider>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QLabel>


Editor::Editor(QString siteId, QWidget *parent) : textEdit(new QTextEdit(this)), cursor(textEdit->textCursor()), siteId(siteId), QMainWindow(parent), ui(new Ui::Editor) {
    ui->setupUi(this);
    setWindowTitle(QCoreApplication::applicationName());
    setCentralWidget(textEdit);

    QTextDocument *doc = textEdit->document();

    // Controller
    connect(doc, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);

}

void Editor::setController(Controller *controller) {
    Editor::controller = controller;
}

void Editor::onTextChanged(int position, int charsRemoved, int charsAdded) {
    std::cout << "POSITION: " << position << " CHARS_ADDED: " << charsAdded << " CHARS_REMOVED: " << charsRemoved << std::endl;

    // check if signal is valid
    bool validSignal = true;

    textEdit->undo();
    QString prevString = textEdit->toPlainText();
    textEdit->redo();
    if(charsAdded == charsRemoved && prevString.size() < charsAdded-1) {
        validSignal = false;
        std::cout << "invalid signal\n";
    }

    QString test = textEdit->toPlainText().mid(position, charsAdded);
    textEdit->textCursor().position();
    if(validSignal && charsAdded == charsRemoved && test.isEmpty()) {
        validSignal = false;
        std::cout << "invalid signal\n";
    }
    // signal validity checked.

    if(validSignal) {
        int currentSize = textEdit->toPlainText().size();
        if(position == 0 && currentSize != charsAdded && charsAdded > 0 && charsRemoved > 0) {
            std::cout << "Paste" << std::endl;
            // esempi:
            // ciaoiao(ciao) --> aggiunti 7, rimossi 4 (ultimi 4)
            // ciaociao(ciao) --> aggiunti 8, rimossi 4 (ultimi 4)
            // ciaoao(ciao) --> aggiunti 6, rimossi 4 (ultimi 4)
            // --> cancello dal modello ciò che c'era prima (ciao) e aggiungo il resto.

            // paste operation... (decrement added)
            charsAdded--;
            charsRemoved--;
            int line, ch;

            if(charsRemoved) {
                // get endPos
                textEdit->undo();
                cursor.setPosition(textEdit->toPlainText().size());
                line = cursor.blockNumber();
                ch = cursor.positionInBlock();
                Pos endPos{ch, line}; // Pos(int ch, int line);
                textEdit->redo();
                // delete everithing before paste...
                this->controller->localDelete(Pos{0, 0}, endPos);
            }


            QString chars = textEdit->toPlainText().mid(position, charsAdded);
            // get start position
            cursor.setPosition(position);
            line = cursor.blockNumber();
            ch = cursor.positionInBlock();
            Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);
            this->controller->localInsert(chars, startPos);

        } else {
            //std::cout << std::endl << "onTextChanged: " << "position = " << position << std::endl;

            if(charsAdded) {
                QString chars = textEdit->toPlainText().mid(position, charsAdded);

                // get start position
                cursor.setPosition(position);
                int line = cursor.blockNumber();
                int ch = cursor.positionInBlock();
                //std::cout << std::endl << "startPos (ch, line): (" << ch << ", " << line << ")" << std::endl << std::endl;
                Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);

                this->controller->localInsert(chars, startPos);
            }

            if(charsRemoved) {
                // get startPos
                int line, ch;
                cursor.setPosition(position);
                line = cursor.blockNumber();
                ch = cursor.positionInBlock();
                Pos startPos{ch, line}; // Pos(int ch, int line);

                // get endPos
                textEdit->undo();
                cursor.setPosition(position + charsRemoved);
                line = cursor.blockNumber();
                ch = cursor.positionInBlock();
                Pos endPos{ch, line}; // Pos(int ch, int line);
                textEdit->redo();

                //std::cout << "chars removed." << std::endl;
                //std::cout << "startPos (ch, line): (" << startPos.getCh() << ", " << startPos.getLine() << ")" << std::endl;
                //std::cout << "endPos (ch, line): (" << endPos.getCh() << ", " << endPos.getLine() << ")" << std::endl << std::endl;

                this->controller->localDelete(startPos, endPos);
            }
        }
    }
}

void Editor::insertChar(char character, Pos pos) {
    QTextCursor oldCursor = cursor;

    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());
    cursor.insertText(QString{character});

    this->cursor = oldCursor;
}

void Editor::deleteChar(Pos pos) {
    QTextCursor oldCursor = cursor;

    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());
    cursor.deleteChar();

    this->cursor = oldCursor;
}

void Editor::on_actionShare_file_triggered()
{

}

void Editor::on_actionOpen_triggered()
{

}

void Editor::on_actionSave_as_PDF_triggered()
{

}

void Editor::on_actionLogout_triggered()
{

}

Editor::~Editor()
{
    delete ui;
}
