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
#include <QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtPrintSupport/qtprintsupportglobal.h>
#include <QPrintDialog>
#include <QPrinter>
#include <QColorDialog>
#include <sharefile.h>
#include <QtCharts>


Editor::Editor(QString siteId, QWidget *parent, Controller *controller) : textEdit(new QTextEdit(this)), textDocument(textEdit->document()),
                                                                          siteId(siteId), QMainWindow(parent), ui(new Ui::Editor), controller(controller),
                                                                          colors({"#B22222",/*"#DC143C","#FF0000","#FF6347","#FF7F50","#CD5C5C","#F08080","#E9967A","#FA8072","#FFA07A","#FF4500","#FF8C00","#FFA500","#FFD700","#B8860B","#DAA520","#EEE8AA","#BDB76B","#F0E68C","#808000","#FFFF00","#9ACD32","#556B2F","#6B8E23","#7CFC00","#7FFF00","#ADFF2F","#006400","#008000","#228B22","#00FF00","#32CD32","#8FBC8F","#00FA9A","#00FF7F","#2E8B57","#66CDAA","#3CB371","#20B2AA","#2F4F4F","#008080","#008B8B","#00FFFF","#00FFFF","#00CED1","#40E0D0","#48D1CC","#AFEEEE","#7FFFD4","#5F9EA0","#4682B4","#6495ED","#00BFFF","#1E90FF","#87CEEB","#87CEFA","#191970","#000080","#00008B","#0000CD","#0000FF","#4169E1","#8A2BE2","#4B0082","#483D8B","#6A5ACD","#7B68EE","#9370DB","#8B008B","#9400D3","#9932CC","#BA55D3","#800080","#DDA0D","#EE82EE","#FF00FF","#DA70D6","#C71585","#DB7093","#FF1493","#FF69B4","#8B4513","#A0522D","#D2691E","#CD853F","#F4A460","#DEB887","#D2B48C","#BC8F8F","#708090","#778899","#B0C4DE",*/"#E6E6FA",
                                                                                 }){

    ui->setupUi(this);
    setWindowTitle(QCoreApplication::applicationName());
    setCentralWidget(textEdit);

    ui->dockWidget->setTitleBarWidget(new QLabel("Online users"));

    ui->userListWidget->resize(this->geometry().width(), this->geometry().height());


    //this->otherCursor.setStyleSheet("background-color : " + color.name(QColor::HexArgb) + ";");


    // TODO: from QByteArray to QPixMap

    #if UI
        QPixmap pix = controller->getUser()->getAvatar();
        int w=ui->avatar->width();
        int h=ui->avatar->height();
        ui->avatar->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
        ui->username->setText(controller->getUser()->getUsername());
        loadingMovie = new QMovie(":/rec/img/loading.gif");
        ui->loading->setMovie(loadingMovie);
        loadingMovie->start();
        ui->editAccount->close();
        ui->loading->show();
    #endif


    this->textCursor = textEdit->textCursor();

    isRedoAvailable = false;

    setupTextActions();

    // Controller
    connect(textDocument, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);

    // cursor position (for change style)
    connect(textEdit, &QTextEdit::cursorPositionChanged,
            this, &Editor::onCursorPositionChanged);


    m_shadowEffect1 = new QGraphicsDropShadowEffect(this);
    m_shadowEffect1->setColor(QColor(0, 0, 0, 255 * 0.1));
    m_shadowEffect1->setXOffset(0);
    m_shadowEffect1->setYOffset(4);
    m_shadowEffect1->setBlurRadius(12);
    m_shadowEffect1->setEnabled(true);
    ui->mainToolBar->setGraphicsEffect(m_shadowEffect1);
}

void Editor::setupTextActions() {
    QToolBar *tb = addToolBar(tr("Format Actions"));
    QMenu *menu = menuBar()->addMenu(tr("F&ormat"));

    tb->setStyleSheet("QToolBar{\n"
                      "border: none;\n"
                      "background: rgb(255, 255, 255);\n"
                      "}\n"
                      "\n"
                      "QToolButton:after{\n"
                      "background-color: rgb(247, 245, 249);\n"
                      "}\n"
                      "\n"
                      "QToolButton:hover{\n"
                      "background-color: rgb(247, 245, 249);\n"
                      "}\n"
                      "\n"
                      "QToolButton:focus{\n"
                      "background-color: rgb(247, 245, 249);\n"
                      "}");
    tb->setMinimumHeight(60);
    //tb->setMaximumHeight(60);

    m_shadowEffect2 = new QGraphicsDropShadowEffect(this);
    m_shadowEffect2->setColor(QColor(0, 0, 0, 255 * 0.1));
    m_shadowEffect2->setXOffset(0);
    m_shadowEffect2->setYOffset(4);
    m_shadowEffect2->setBlurRadius(12);
    // hide shadow
    m_shadowEffect2->setEnabled(true);
    tb->setGraphicsEffect(m_shadowEffect2);


    // bold
    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(":/rec/img/bold.png"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &Editor::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    menu->addSeparator();

    // italic
    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(":/rec/img/italic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &Editor::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    // underline
    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(":/rec/img/underline.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Underline"), this, &Editor::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(":/rec/img/align-left.png"));
    actionAlignLeft = new QAction(leftIcon, tr("&Left"), this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(":/rec/img/align-center.png"));
    actionAlignCenter = new QAction(centerIcon, tr("C&enter"), this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(":/rec/img/align-right.png"));
    actionAlignRight = new QAction(rightIcon, tr("&Right"), this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(":/rec/img/align.png"));
    actionAlignJustify = new QAction(fillIcon, tr("&Justify"), this);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    QActionGroup *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &Editor::textAlign);

    if (QApplication::isLeftToRight()) {
        alignGroup->addAction(actionAlignLeft);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignRight);
    } else {
        alignGroup->addAction(actionAlignRight);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignLeft);
    }
    alignGroup->addAction(actionAlignJustify);

    tb->addActions(alignGroup->actions());
    menu->addActions(alignGroup->actions());


    menu->addSeparator();


    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = menu->addAction(pix, tr("&Color..."), this, &Editor::textColor);
    tb->addAction(actionTextColor);

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);

    connect(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &Editor::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
            foreach (int size, standardSizes)
            comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &Editor::textSize);


    //QFont dFont(QString("Al Bayan"), 12);

   // this->textEdit->document()->setDefaultFont(dFont);

}

void Editor::textBold() {
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void Editor::textUnderline() {
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void Editor::textItalic() {
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void Editor::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void Editor::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void Editor::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void Editor::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}



void Editor::textAlign(QAction *a)
{
   // int at=LEFT;
    int alCode=0;

    if (a == actionAlignLeft) {
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute); //Absolute means that the "left" not depends on layout of widget
        alCode=textEdit->alignment();
        qDebug() << alCode;
    }
    else if (a == actionAlignCenter) {
        textEdit->setAlignment(Qt::AlignHCenter);
       // at = CENTER;
        alCode=textEdit->alignment();
        qDebug() << alCode;
    }
    else if (a == actionAlignRight) {
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
      // at = RIGHT;
        alCode=textEdit->alignment();
        qDebug() << alCode;
    }
    else if (a == actionAlignJustify) {
        textEdit->setAlignment(Qt::AlignJustify);
       // at=JUSTIFY;
        alCode=textEdit->alignment();
        qDebug() << alCode;
    }

    int oldCursorPos = this->textCursor.position();
    int start=this->startSelection;
    int end=this->endSelection;

    this->textCursor.setPosition(start);
    int startBlock=this->textCursor.blockNumber();

    this->textCursor.setPosition(end);
    int endBlock=this->textCursor.blockNumber();

    for(int blockNum=startBlock; blockNum<=endBlock; blockNum++) {
        this->controller->alignChange(alCode, blockNum);
    }

}

void Editor::remoteAlignmentChanged(int alignment, int blockNumber){

    int oldCursorPos = this->textCursor.position();

    int bc = this->textEdit->textCursor().document()->blockCount();

    this->textCursor.movePosition(QTextCursor::Start);
    this->textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, blockNumber);

    int cursorPos = this->textCursor.position();
    int num=this->textCursor.blockNumber();

    QTextBlockFormat f=this->textCursor.blockFormat();

    f.setAlignment(Qt::Alignment(alignment));
    this->textCursor.setBlockFormat(f);

//    if (at == LEFT) {
//        f.setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
//        this->textCursor.setBlockFormat(f);
//    }
//    else if (at == CENTER) {
//        f.setAlignment(Qt::AlignHCenter);
//        this->textCursor.setBlockFormat(f);
//    }
//    else if (at == RIGHT) {
//    f.setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
//        this->textCursor.setBlockFormat(f);
//    }
//    else if (at == JUSTIFY) {
//        f.setAlignment(Qt::AlignJustify);
//        this->textCursor.setBlockFormat(f);
//    }

    this->textCursor.setPosition(oldCursorPos);
}



void Editor::formatText(std::vector<int> styleBlocks){



    QTextDocument *doc = textEdit->document();


    disconnect(doc, &QTextDocument::contentsChange,
               this, &Editor::onTextChanged);



    for(int i=0; i<styleBlocks.size(); i++){
        this->remoteAlignmentChanged(styleBlocks.at(i),i);
    }

     connect(doc, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);

}

void Editor::mergeFormatOnWordOrSelection(const QTextCharFormat &format) {
    QTextCursor cursor = textEdit->textCursor();
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);

    // updateUI
    actionTextBold->setChecked(format.fontWeight() == QFont::Bold);
    actionTextItalic->setChecked(format.fontItalic());
    actionTextUnderline->setChecked(format.fontUnderline());
    colorChanged(format.foreground().color());
    comboFont->setCurrentFont(format.font());
    int index = comboSize->findText(QString::number(format.fontPointSize()));
    if(index != -1) {
        comboSize->setCurrentIndex(index);
    }
}

void Editor::setController(Controller *controller) {
    Editor::controller = controller;
}

void Editor::onTextChanged(int position, int charsRemoved, int charsAdded) {
    disconnect(textEdit, &QTextEdit::cursorPositionChanged,
            this, &Editor::onCursorPositionChanged);

    qDebug() << "editor.cpp - onTextChanged()     position: " << position << " chars added: " << charsAdded << " chars removed: " << charsRemoved;

    saveCursor();

    if(validSignal(position, charsAdded, charsRemoved)) {
        //qDebug() << "VALID SIGNAL";
        //std::cout << "VALID SIGNAL" << std::endl;

        // it is possible that user change only the style or the user re-paste the same letters... check it
        QString textAdded = textEdit->toPlainText().mid(position, charsAdded);
        undo();
        QString textRemoved = textEdit->toPlainText().mid(position, charsAdded);
        redo();
        if(charsAdded == charsRemoved && textAdded == textRemoved) {
            // qDebug() << "text doesn't change (maybe style changed)";
            if(position == 0 && textDocument->characterCount()-1 != charsAdded) {
                // correction when paste something in first position.
                charsAdded--;
            }
            QTextCursor cursor = textEdit->textCursor();

            for(int i=0; i<charsAdded; i++) {
                // for each char added
                cursor.setPosition(position + i);
                int line = cursor.blockNumber();
                int ch = cursor.positionInBlock();
                Pos pos{ch, line}; // Pos(int ch, int line, const std::string);
                // select char
                cursor.setPosition(position + i + 1, QTextCursor::KeepAnchor);

                QTextCharFormat textCharFormat = cursor.charFormat();

                this->controller->styleChange(textCharFormat, pos);
            }
        } else {
            if(position == 0 && charsAdded > 0 && charsRemoved > 0) {
                // correction when paste something in first position.
                charsAdded--;
                charsRemoved--;
            }

            if(charsRemoved) {
                if(isRedoAvailable) {
                    // chars removed due to undo operation.

                    // get endPos
                    textCursor.setPosition(position);
                    int line = textCursor.blockNumber();
                    int ch = textCursor.positionInBlock();
                    Pos startPos{ch, line}; // Pos(int ch, int line);

                    // get startPos
                    redo();
                    textCursor.setPosition(position + charsRemoved);
                    line = textCursor.blockNumber();
                    ch = textCursor.positionInBlock();
                    Pos endPos{ch, line}; // Pos(int ch, int line);
                    undo();

                    //qDebug() << "DELETING: startPos: (" << startPos.getLine() << ", " << startPos.getCh() << ") - endPos: ("  << endPos.getLine() << ", " << endPos.getCh() << ")";
                    this->controller->localDelete(startPos, endPos);
                } else {
                    // get startPos
                    int line, ch;
                    textCursor.setPosition(position);
                    line = textCursor.blockNumber();
                    ch = textCursor.positionInBlock();
                    Pos startPos{ch, line}; // Pos(int ch, int line);

                    // get endPos
                    undo();
                    textCursor.setPosition(position + charsRemoved);
                    line = textCursor.blockNumber();
                    ch = textCursor.positionInBlock();
                    Pos endPos{ch, line}; // Pos(int ch, int line);
                    redo();

                    //qDebug() << "DELETING: startPos: (" << startPos.getLine() << ", " << startPos.getCh() << ") - endPos: ("  << endPos.getLine() << ", " << endPos.getCh() << ")";
                    this->controller->localDelete(startPos, endPos);
                }
            }

            if(charsAdded) {
                QTextCursor cursor = textEdit->textCursor();
                QString chars = textEdit->toPlainText().mid(position, charsAdded);

                for(int i=0; i<charsAdded; i++) {
                    // for each char added
                    cursor.setPosition(position + i);
                    int line = cursor.blockNumber();
                    int ch = cursor.positionInBlock();
                    Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);
                    // select char
                    cursor.setPosition(position + i + 1, QTextCursor::KeepAnchor);
                    QTextCharFormat charFormat = cursor.charFormat();
                    
                    this->controller->localInsert(chars.at(i), charFormat, startPos);
                }
            }
        }

        restoreCursor();
        onCursorPositionChanged();
    } else {
        qDebug() << " "; // new Line
        //qDebug() << "INVALID SIGNAL";
        //std::cout << "INVALID SIGNAL" << std::endl;
        if(this->startSelection != this->endSelection) {
            // text was selected... restore the selction
            restoreCursorSelection();
        } else {
            restoreCursor();
        }
    }
    connect(textEdit, &QTextEdit::cursorPositionChanged,
            this, &Editor::onCursorPositionChanged);

    qDebug() << this->textEdit->document()->blockCount();
}

void Editor::insertChar(char character, QTextCharFormat textCharFormat, Pos pos, QString siteId) {
    int oldCursorPos = textCursor.position();

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
    textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());


    QTextDocument *doc = textEdit->document();
    disconnect(doc, &QTextDocument::contentsChange,
               this, &Editor::onTextChanged);

    textCursor.insertText(QString{character});

    this->otherCursors[siteId]->setOtherCursorPosition(textCursor.position());

    qDebug() << "Pos text cursor (after insert): " << textCursor.position();
    qDebug() << "Pos other text cursor (after insert): " << this->otherCursors[siteId]->getOtherCursor().position();


    textCursor.setPosition(textCursor.position()-1, QTextCursor::KeepAnchor);
    textCursor.mergeCharFormat(textCharFormat);
    textEdit->mergeCurrentCharFormat(textCharFormat);


    qDebug()<<siteId;
//    this->otherCursors[siteId]->setOtherCursorPosition(textCursor.position());
//
//    Pos coord(textCursor.positionInBlock(), textCursor.blockNumber());
//
//    this->updateCursor(coord, siteId);

    //qDebug() << "Position of OTHER CURSOR: " << otherTextCursor.position();

    connect(doc, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);


    textCursor.setPosition(oldCursorPos);
}


void Editor::updateCursor(Pos position, QString siteId){

    disconnect(textEdit, &QTextEdit::cursorPositionChanged,
               this, &Editor::onCursorPositionChanged);

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, position.getLine());
    textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position.getCh());

//    QRect coord=this->textEdit->cursorRect(textCursor);
//
//    this->otherCursors[siteId]->move(coord.topRight().x()+7,coord.topRight().y()-10);
//
//    this->otherCursors[siteId]->show();

    connect(textEdit, &QTextEdit::cursorPositionChanged,
            this, &Editor::onCursorPositionChanged);


}

void Editor::changeStyle(Pos pos, const QTextCharFormat &textCharFormat) {
    //qDebug() << "bold" << format.isBold();
    //qDebug() << "underline" << format.isUnderline();
    //qDebug() << "italic" << format.isItalic();
    int oldCursorPos = textCursor.position();


    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
    textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());

    QTextDocument *doc = textEdit->document();

    disconnect(doc, &QTextDocument::contentsChange,
               this, &Editor::onTextChanged);

    textCursor.setPosition(textCursor.position()+1, QTextCursor::KeepAnchor);
    textCursor.mergeCharFormat(textCharFormat);
    textEdit->mergeCurrentCharFormat(textCharFormat);
    mergeFormatOnWordOrSelection(textCharFormat);

    connect(doc, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);

    textCursor.setPosition(oldCursorPos);
}



void Editor::deleteChar(Pos pos, QString siteId) {

    //TODO: il siteId non è di chi manda il messaggio ma di chi ha inserito il carattere

    int oldCursorPos = textCursor.position();

    textCursor.movePosition(QTextCursor::Start);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
    textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());

    QTextDocument *doc = textEdit->document();
    disconnect(doc, &QTextDocument::contentsChange,
               this, &Editor::onTextChanged);

    textCursor.deleteChar();
    
    if( (textCursor.position()-1)<0 ) {
        this->otherCursors[siteId]->setOtherCursorPosition( 0 );
    }
    else{
        this->otherCursors[siteId]->setOtherCursorPosition( textCursor.position()-1 );
    }
    
    qDebug() << "Pos text cursor (after delete): " << textCursor.position();
    qDebug() << "Pos other text cursor (after delete): " << this->otherCursors[siteId]->getOtherCursor().position();

    qDebug() << siteId;

//    if(!this->otherCursors[siteId].isNull()) {
//        this->otherCursors[siteId]->setOtherCursorPosition(textCursor.position());
//
//
////    qDebug() << "Position OTHER CURSOR " << this->otherCursors[siteId]->getOtherCursor().position();
//        Pos coord(textCursor.positionInBlock(), textCursor.blockNumber());
//
//        this->updateCursor(coord, siteId);
//
//    }

    connect(doc, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);

    textCursor.setPosition(oldCursorPos);
}

void Editor::setFormat(CharFormat charFormat) {
    QTextCharFormat fmt = charFormat.toTextCharFormat();

    mergeFormatOnWordOrSelection(fmt);
}

void Editor::onCursorPositionChanged() {
    QTextCursor cursor = textEdit->textCursor();
    if(!cursor.hasSelection()) {
        int cursorPos = cursor.position();
        if(cursorPos == 0) {
            setFormat(CharFormat()); // defaul character
        } else if(cursorPos > 0) {
            cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
            QTextCharFormat textCharFormat = cursor.charFormat();
            setFormat(CharFormat(textCharFormat.fontWeight() == QFont::Bold,
                                 textCharFormat.fontItalic(),
                                 textCharFormat.fontUnderline(),
                                 textCharFormat.foreground().color(),
                                 textCharFormat.font().family(),
                                 textCharFormat.fontPointSize()));
        }
    }
}

void Editor::on_actionNew_File_triggered() {
    QMessageBox::information(this, "File", "File!");
}

void Editor::on_actionShare_file_triggered() {
    ShareFile *shareFile = new ShareFile(this, *this->filename);
    shareFile->show();
}

void Editor::on_actionOpen_triggered() {
    emit showFinder();
}

void Editor::on_actionSave_as_PDF_triggered() {
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    textEdit->document()->print(&printer);
//	statusBar()->showMessage(tr("Exported \"%1\"").arg(QDir::toNativeSeparators(fileName)));

    QMessageBox::information(this, "PDF", "File Esportato");
}

void Editor::on_actionLogout_triggered() {
    emit logout();
}

Editor::~Editor() {
    delete ui;
}

void Editor::undo() {
    disconnect(textDocument, &QTextDocument::contentsChange,
               this, &Editor::onTextChanged);
    textEdit->undo();
    connect(textDocument, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);
}

void Editor::redo() {
    disconnect(textDocument, &QTextDocument::contentsChange,
               this, &Editor::onTextChanged);
    textEdit->redo();
    connect(textDocument, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);
}

bool Editor::validSignal(int position, int charsAdded, int charsRemoved) {
    // check if signal is valid
    bool validSignal = true;
    isRedoAvailable = textDocument->isRedoAvailable();

    QString textAdded = textEdit->toPlainText().mid(position, charsAdded);
    undo();
    QString textRemoved = textEdit->toPlainText().mid(position, charsAdded);
    redo();
    if(charsAdded == charsRemoved && position+charsAdded > textDocument->characterCount()-1 && textAdded == textRemoved) {
        // wrong signal when apply style in editor 1 (line 2) and then write something in editor2
        qDebug() << "WRONG SIGNAL 1";
        validSignal = false;
    }

    int currentDocumentSize = textDocument->characterCount()-1;
    undo();
    int undoDocumentSize = textDocument->characterCount()-1;
    redo();
    //qDebug() << "currentDocumentSize" << currentDocumentSize << " undoDocumentSize" <<undoDocumentSize;
    if(validSignal && charsAdded == charsRemoved && currentDocumentSize != (undoDocumentSize + charsAdded - charsRemoved)) {
        // wrong signal when editor gets focus and something happen.
        //qDebug() << "WRONG SIGNAL 2";
        validSignal = false;
    }

    /*if(validSignal && charsAdded == charsRemoved && (position+charsRemoved) > (textDocument->characterCount()-1)) {
        //qDebug() << "WRONG SIGNAL";
        // wrong signal when client add new line after it takes focus or when it move the cursor in the editor after the focus acquired
        validSignal = false;
    }*/

    QString test = textEdit->toPlainText().mid(position, charsAdded);
    if(validSignal && charsAdded == charsRemoved && test.isEmpty()) {
        // wrong signal when editor opens.
        //qDebug() << "WRONG SIGNAL 3";
        validSignal = false;
    }

    // check if text selected
    bool textSelected = false;
    int beginPos = textEdit->textCursor().selectionStart();
    int endPos = textEdit->textCursor().selectionEnd();
    int currentSize = textEdit->toPlainText().size();
    if(beginPos != endPos) {
        textSelected = true;
    }
    if(validSignal && textSelected && charsAdded == charsRemoved && currentSize != 0) {
        // this solve the bug when we select text (in multilines), when the textedit has not the fucus, and we delete them.
        //qDebug() << "WRONG SIGNAL 4";
        validSignal = false;
    }

    return validSignal;
}

void Editor::resizeEvent(QResizeEvent *event) {
    ui->userListWidget->resize(149, textEdit->geometry().height() - 18 -100);

    ui->userWidget->setGeometry(0, textEdit->geometry().height() - 18 -100, ui->userWidget->width(), ui->userWidget->height());
}

void Editor::removeUser(QString user) {
    users.erase(std::remove_if(users.begin(), users.end(), [user](const QString &s) {
        return s == user;
    }));

   // qDebug() << "Before: " << this->otherCursors.size();
   // qDebug() << this->otherCursors[user]->text();

    ui->userListWidget->clear();
    ui->userListWidget->addItems(users);
    


  //  qDebug() << "After: " << this->otherCursors.size();



}

void Editor::setUsers(QStringList users) {
    #if UI
        if (loadingFlag){
            loadingMovie->stop();
            ui->loading->close();
            loadingFlag = false;
            ui->editAccount->show();
        }
    #else
    QPixmap pix;
    pix.load(":/icons/user_icon.jpg");
    int w=ui->avatar->width();
    int h=ui->avatar->height();
    ui->avatar->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
    ui->username->setText("Debug");
#endif
    this->users = users;
    ui->userListWidget->addItems(users);
    controller->stopLoadingPopup();

    std::for_each( users.begin(), users.end(), [this](QString user){
        int colorIndex=qHash(user)%2;
        QColor color(colors[colorIndex]);
        color.setAlpha(128); // opacity
        otherCursors.insert(user, new OtherCursor(this->textDocument, color, Character()));
    });

    qDebug() << this->otherCursors.size();
}

void Editor::saveCursor() {
    QTextCursor cursor = this->textEdit->textCursor();
    this->cursorPos = cursor.position();
    this->startSelection = cursor.selectionStart();
    this->endSelection = cursor.selectionEnd();
}

void Editor::restoreCursor() {
    QTextCursor cursor = this->textCursor;
    cursor.setPosition(this->cursorPos, QTextCursor::MoveAnchor);
    textEdit->setTextCursor(cursor);
}

void Editor::restoreCursorSelection() {
    QTextCursor cursor = this->textCursor;
    cursor.setPosition(this->startSelection);
    cursor.setPosition(this->endSelection, QTextCursor::KeepAnchor);
    textEdit->setTextCursor(cursor);
}

void Editor::replaceText(const QString initialText) {
    QTextDocument *doc = textEdit->document();

    disconnect(doc, &QTextDocument::contentsChange,
               this, &Editor::onTextChanged);

    textEdit->setText(initialText);




    connect(doc, &QTextDocument::contentsChange,
            this, &Editor::onTextChanged);

    QTextCursor newCursor = textEdit->textCursor();
    newCursor.movePosition(QTextCursor::End);
    textEdit->setTextCursor(newCursor);
    qDebug() << "Alignment: " << textEdit->alignment();
}

void Editor::reset() {
    ui->userListWidget->clear();
    QPixmap pix = controller->getUser()->getAvatar();
    int w = ui->avatar->width();
    int h = ui->avatar->height();
    ui->avatar->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
    ui->username->setText(controller->getUser()->getUsername());
    ui->loading->show();
}

void Editor::showError(){
    QMessageBox::information(this, "Error", "Error!");
}

void Editor::setFilename(QString *filename){
    this->filename = filename;
}


