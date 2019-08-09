//
// Created by simone on 05/08/19.
//

#include "Editor.h"


Editor::Editor(std::string siteId, QWidget *parent) : textEdit(new QTextEdit(this)), cursor(textEdit->textCursor()), siteId(siteId), QMainWindow(parent) {
    setWindowTitle(QCoreApplication::applicationName());
    setCentralWidget(textEdit);

    QTextDocument *doc = textEdit->document();

    // Controller
    connect(doc, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);

    // Prove
    /*
    textEdit->append("QUESTA è UNA PROVA");
    textEdit->append("TEXT EDITOR");

    QTextCursor cursor;

    cursor = textEdit->textCursor();
    cursor.setPosition(2);
    cursor.deleteChar();
    textEdit->setTextCursor(cursor);

    cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 2);
    cursor.deleteChar();
    textEdit->setTextCursor(cursor);

    cursor = textEdit->textCursor();
    cursor.setPosition(2);
    cursor.setPosition(8, QTextCursor::KeepAnchor);
    textEdit->setTextCursor(cursor);


    cursor = textEdit->textCursor();
    cursor.setPosition(20);
    cursor.deleteChar();
    textEdit->setTextCursor(cursor);


    cursor = textEdit->textCursor();
    cursor.setPosition(20);
    cursor.insertText("_P_");
    textEdit->setTextCursor(cursor);
    */

}

void Editor::setController(Controller *controller) {
    Editor::controller = controller;
}

void Editor::onTextChanged(int position, int charsRemoved, int charsAdded) {
    if(charsAdded == charsRemoved) {
        /* TODO risolvere questo BUG:
         * intercetta un segnale (errato) quando si toglie il focus dall'editor e poi si rimette il focus e si sposta il cursore
         * oppure appena si apre l'editor. Però è possibile rimuovere e inserire (sostituire) un carattere in posizione 0.
         * il signal errato ha come parametri position=(first line position); charsRemoved=charsAdded*/
        // std::cout << "POSITION: " << position << " CHARS_REMOVED: " << charsRemoved << " CHARS_ADDED: " << charsAdded << std::endl;
    } else {
        std::cout << std::endl << "onTextChanged: " << "position = " << position << std::endl;

        if(charsAdded) {
            QString chars = textEdit->toPlainText().mid(position, charsAdded);

            std::vector<char> charsVector;
            std::cout << "char(s): ";
            for (QChar c : chars) {
                charsVector.push_back(c.toLatin1());
                if (c.toLatin1() == '\n') std::cout << "\\n ";
                else std::cout << c.toLatin1() << " ";
            }

            // get start position
            cursor.setPosition(position);
            int line = cursor.blockNumber();
            int ch = cursor.positionInBlock();
            std::cout << std::endl << "startPos (ch, line): (" << ch << ", " << line << ")" << std::endl << std::endl;
            Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);

            this->controller->localInsert(charsVector, startPos);
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

            std::cout << "chars removed." << std::endl;
            std::cout << "startPos (ch, line): (" << startPos.getCh() << ", " << startPos.getLine() << ")" << std::endl;
            std::cout << "endPos (ch, line): (" << endPos.getCh() << ", " << endPos.getLine() << ")" << std::endl << std::endl;

            this->controller->localDelete(startPos, endPos);
        }
    }
}

const std::string &Editor::getSiteId() const {
    return siteId;
}

QTextEdit *Editor::getTextEdit() const {
    return textEdit;
}
