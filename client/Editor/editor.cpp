//
// Created by simone on 05/08/19.
//

#include "editor.h"
#include "ui_editor.h"
#include "../../server/SimpleCrypt/SimpleCrypt.h"
#include <QMenu>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QColorDialog>
#include <../ui/ShareFile/sharefile.h>
#include <QMetaObject>
#include <QMetaType>

Q_DECLARE_METATYPE(QTextCursor *);

Q_DECLARE_METATYPE(QTextCursor);


Editor::Editor(QString siteId, QWidget *parent, Controller *controller) : textEdit(new TextEdit(this, controller)),
																		  textDocument(textEdit->document()),
																		  siteId(siteId), QMainWindow(parent),
																		  ui(new Ui::Editor), controller(controller),
																		  colors({"#ff4500", "#7fffd4", "#deb887",
																				  "#00ffff", "#ff7f50", "#0000ff",
																				  "#9932cc", "#ff1493", "#ffd700",
																				  "#a52a2a", "#1e90ff", "#9370db",
																				  "#006400", "#ff0000", "#008080"}) {

	/* Setup UI */
	ui->setupUi(this);
	setWindowTitle(QCoreApplication::applicationName());
	setCentralWidget(textEdit);
	ui->dockWidget->setTitleBarWidget(new QLabel("Online users"));
	ui->userListWidget->resize(this->geometry().width(), this->geometry().height());

	colorIndex = 0;

	/* Handle user in the editor part */
	changeUser();
	ui->loading->show();
	connect(this->controller, SIGNAL(userRecived()), this, SLOT(changeUser()));

	this->textCursor = textEdit->textCursor();

	isRedoAvailable = false;

	setupTextActions();

	// Controller
	connect(textDocument, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	// cursor position (for change style)
	connect(textEdit, &QTextEdit::cursorPositionChanged,
			this, &Editor::onCursorPositionChanged);


	/* adding shadow effect */
	m_shadowEffect1 = new QGraphicsDropShadowEffect(this);
	m_shadowEffect1->setColor(QColor(0, 0, 0, 255 * 0.1));
	m_shadowEffect1->setXOffset(0);
	m_shadowEffect1->setYOffset(4);
	m_shadowEffect1->setBlurRadius(12);
	m_shadowEffect1->setEnabled(true);
	ui->mainToolBar->setGraphicsEffect(m_shadowEffect1);

	connect(ui->userListWidget, SIGNAL(itemClicked(QListWidgetItem * )),
			this, SLOT(onListUsersItemClicked(QListWidgetItem * )));

    connect(ui->offlineTextButton, SIGNAL(clicked()), this, SLOT(handleOfflineText()));


}

void Editor::setupTextActions() {
	QToolBar *tbCopyCutPaste = addToolBar(tr("CopyCutPaste Actions"));
	QMenu *menuCopyCutPaste = menuBar()->addMenu(tr("E&dit"));
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
	tbCopyCutPaste->setStyleSheet("QToolBar{\n"
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
	tbCopyCutPaste->setMinimumHeight(60);

	m_shadowEffect2 = new QGraphicsDropShadowEffect(this);
	m_shadowEffect2->setColor(QColor(0, 0, 0, 255 * 0.1));
	m_shadowEffect2->setXOffset(0);
	m_shadowEffect2->setYOffset(4);
	m_shadowEffect2->setBlurRadius(12);
	// hide shadow
	m_shadowEffect2->setEnabled(true);
	tb->setGraphicsEffect(m_shadowEffect2);
	m_shadowEffect3 = new QGraphicsDropShadowEffect(this);
	m_shadowEffect3->setColor(QColor(0, 0, 0, 255 * 0.1));
	m_shadowEffect3->setXOffset(0);
	m_shadowEffect3->setYOffset(4);
	m_shadowEffect3->setBlurRadius(12);
	// hide shadow
	m_shadowEffect3->setEnabled(true);
	tbCopyCutPaste->setGraphicsEffect(m_shadowEffect3);

	// Copy
	const QIcon copyIcon = QIcon::fromTheme("Copy", QIcon(":/rec/img/copy2.png"));
	actionCopy = menuCopyCutPaste->addAction(copyIcon, tr("&Copy"), this, &Editor::textCopy);
	tbCopyCutPaste->addAction(actionCopy);

	// Cut
	const QIcon cutIcon = QIcon::fromTheme("Cut", QIcon(":/rec/img/cut.png"));
	actionCut = menuCopyCutPaste->addAction(cutIcon, tr("&Cut"), this, &Editor::textCut);
	tbCopyCutPaste->addAction(actionCut);

	// Paste
	const QIcon pasteIcon = QIcon::fromTheme("Paste", QIcon(":/rec/img/paste.png"));
	actionPaste = menuCopyCutPaste->addAction(pasteIcon, tr("&Paste"), this, &Editor::textPaste);
	tbCopyCutPaste->addAction(actionPaste);

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
			foreach(int
							size, standardSizes)comboSize->addItem(QString::number(size));
	comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

	connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &Editor::textSize);

	this->updateAlignmentPushButton();
	QFont dFont(QString("Arial"), 12);

	this->textEdit->document()->setDefaultFont(dFont);
	qDebug() << "QFont default" << this->textEdit->textCursor().charFormat().font();

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

void Editor::textFamily(const QString &f) {
	QTextCharFormat fmt;
	fmt.setFontFamily(f);
	mergeFormatOnWordOrSelection(fmt);
}

void Editor::textSize(const QString &p) {
	qreal pointSize = p.toFloat();
	if (p.toFloat() > 0) {
		QTextCharFormat fmt;
		fmt.setFontPointSize(pointSize);
		mergeFormatOnWordOrSelection(fmt);
	}
}

void Editor::textColor() {
	QColor col = QColorDialog::getColor(textEdit->textColor(), this);
	if (!col.isValid())
		return;
	QTextCharFormat fmt;
	fmt.setForeground(col);
	mergeFormatOnWordOrSelection(fmt);
	colorChanged(col);
}

void Editor::colorChanged(const QColor &c) {
	QPixmap pix(16, 16);
	pix.fill(c);
	actionTextColor->setIcon(pix);
}


void Editor::textAlign(QAction *a) {
	int alCode = 0;

	if (a == actionAlignLeft) {
		textEdit->setAlignment(
				Qt::AlignLeft | Qt::AlignAbsolute); //Absolute means that the "left" not depends on layout of widget
		alCode = textEdit->alignment();
		qDebug() << alCode;
	} else if (a == actionAlignCenter) {
		textEdit->setAlignment(Qt::AlignHCenter);
		alCode = textEdit->alignment();
		qDebug() << alCode;
	} else if (a == actionAlignRight) {
		textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
		alCode = textEdit->alignment();
		qDebug() << alCode;
	} else if (a == actionAlignJustify) {
		textEdit->setAlignment(Qt::AlignJustify);
		alCode = textEdit->alignment();
		qDebug() << alCode;
	}

	int start = this->startSelection;
	int end = this->endSelection;

	this->textCursor.setPosition(start);
	int startBlock = this->textCursor.blockNumber();

	this->textCursor.setPosition(end);
	int endBlock = this->textCursor.blockNumber();
	for (int blockNum = startBlock; blockNum <= endBlock; blockNum++) {
		controller->getCrdt()->alignChange(alCode, blockNum);
		qDebug() << alCode << blockNum;
	}

}

void Editor::remoteAlignmentChanged(int alignment, int blockNumber) {
	disconnect(textDocument, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);
	int oldCursorPos = textCursor.position();

	int bc = this->textEdit->textCursor().document()->blockCount();

	textCursor.movePosition(QTextCursor::Start);
	textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, blockNumber);

	int cursorPos = this->textCursor.position();
	int num = this->textCursor.blockNumber();

	QTextBlockFormat f = QTextBlockFormat{textCursor.blockFormat()};

	f.setAlignment(Qt::Alignment(alignment));

	textCursor.setBlockFormat(f);

	textCursor.setPosition(oldCursorPos);
	connect(textDocument, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);
	this->updateAlignmentPushButton();
	this->updateOtherCursorPosition();
}


void Editor::formatText(std::vector<int> styleBlocks) {

	QTextDocument *doc = textEdit->document();


	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);

	qDebug() << styleBlocks.size();

	for (int i = 0; i < styleBlocks.size(); i++) {
		this->remoteAlignmentChanged(styleBlocks.at(i), i);
		qDebug() << "Alignment:" << styleBlocks.at(i) << "Block:" << i;
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
	if (index != -1) {
		comboSize->setCurrentIndex(index);
	}
}

void Editor::setController(Controller *controller) {
	Editor::controller = controller;
}

void Editor::onTextChanged(int position, int charsRemoved, int charsAdded) {
	disconnect(textEdit, &QTextEdit::cursorPositionChanged,
			   this, &Editor::onCursorPositionChanged);

	qDebug() << "editor.cpp - onTextChanged()     position: " << position << " chars added: " << charsAdded
			 << " chars removed: " << charsRemoved;

	qDebug() << "PlainText" << textEdit->toPlainText();
//
	saveCursor();
	QString textAdded;
	try {
		if (validSignal(position, charsAdded, charsRemoved)) {

			// it is possible that user change only the style or the user re-paste the same letters... check it
			textAdded = textEdit->toPlainText().mid(position, charsAdded);
			undo();
			QString textRemoved = textEdit->toPlainText().mid(position, charsAdded);
			redo();
			if (charsAdded == charsRemoved && textAdded == textRemoved) {
				// qDebug() << "text doesn't change (maybe style changed)";
				if (position == 0 && textDocument->characterCount() - 1 != charsAdded) {
					// correction when paste something in first position.
					charsAdded--;
				}
				QTextCursor cursor = textEdit->textCursor();

				controller->getCrdt()->setIsWorking(true);
				controller->getCrdt()->setNumJobs(controller->getCrdt()->getNumJobs() + charsAdded);

				if (cursorPos != startSelection) { // Selection forward
                    if(position == 0 && textDocument->characterCount() - 1 != charsAdded){
                        charsAdded++;
                    }
					for (int i = 0; i < charsAdded; i++) {
						// for each char added
						cursor.setPosition(position + i);
						int line = cursor.blockNumber();
						int ch = cursor.positionInBlock();
						Pos pos{ch, line}; // Pos(int ch, int line, const std::string);
						// select char
						cursor.setPosition(position + i + 1, QTextCursor::KeepAnchor);

						QTextCharFormat textCharFormat = cursor.charFormat();

						controller->getCrdt()->localStyleChange(textCharFormat, pos);
					}
				} else { // Selection backward
                    if(position == 0 && textDocument->characterCount() - 1 != charsAdded){
                        charsAdded++;
                    }

					for (int i = charsAdded - 1; i >= 0; i--) {
						// for each char added
						cursor.setPosition(position + i);
						int line = cursor.blockNumber();
						int ch = cursor.positionInBlock();
						Pos pos{ch, line}; // Pos(int ch, int line, const std::string);
						// select char
						cursor.setPosition(position + i + 1, QTextCursor::KeepAnchor);

						QTextCharFormat textCharFormat = cursor.charFormat();

						controller->getCrdt()->localStyleChange(textCharFormat, pos);
					}
				}


			} else {
				if (position == 0 && charsAdded > 0 && charsRemoved > 0 && copyFlag == true) {
					// correction when paste something in first position.
					/*charsAdded--;
					charsRemoved--;*/
				}
				if (copyFlag)
					copyFlag = false;

				if (charsRemoved) {
					if (isRedoAvailable) {
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
						//emit localDelete(startPos, endPos);
						controller->getCrdt()->setIsWorking(true);
						controller->getCrdt()->setNumJobs(controller->getCrdt()->getNumJobs() + 1);
						controller->getCrdt()->localDelete(startPos, endPos);
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
						controller->getCrdt()->setIsWorking(true);
						controller->getCrdt()->setNumJobs(controller->getCrdt()->getNumJobs() + 1);
						controller->getCrdt()->localDelete(startPos, endPos);
					}
				}
				qDebug() << "PlainText" << textEdit->toPlainText();

				if (charsAdded) {
					QTextCursor cursor = textEdit->textCursor();
					QString chars = textEdit->toPlainText().mid(position, charsAdded);
					bool charsIsEmpty = chars.isEmpty();
					if (chars.isEmpty() && !textAdded.isEmpty()) {
						chars = textAdded;
					}
					qDebug() << "Editor: " << QThread::currentThreadId();

					if (charsAdded == 1) {
						controller->getCrdt()->setIsWorking(true);
						controller->getCrdt()->setNumJobs(controller->getCrdt()->getNumJobs() + 1);
						if (count != pendingChar.size()) {
							count = pendingChar.size();
						}

						cursor.setPosition(position);
						int line = cursor.blockNumber();
						int ch = cursor.positionInBlock();
						Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);
						// select char
						cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
						QTextCharFormat charFormat = cursor.charFormat();
						//emit localInsert(chars.at(i), charFormat, startPos);
						if (charsRemoved == 0) {
							controller->getCrdt()->localInsert(chars.at(0), charFormat, startPos);
						} else {
							controller->getCrdt()->localInsert(chars.at(0), charFormat, startPos, charsIsEmpty);
						}
						this->unsetCharacterColorLocally(startPos, controller->getCrdt()->getSiteId());


					} else {
						controller->getCrdt()->setIsWorking(true);
						controller->getCrdt()->setNumJobs(controller->getCrdt()->getNumJobs() + charsAdded);
						//controller->getCrdt()->copy = true;
						std::cout << "Position Cursor: " << textEdit->position << " " << position;

						if (charsIsEmpty && textDocument->isEmpty()) {
							int line = 0;
							int k = 0;
							cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
							QTextCharFormat charFormat = cursor.charFormat();
							for (int i = 0; i < charsAdded; i++) {
								qDebug() << cursor.position();
								int ch = k;
								Pos startPos{ch, line};
								controller->getCrdt()->localInsert(chars.at(i), charFormat, startPos,
																   i == charsAdded - 1);
								if (chars.at(i) == '\n') {
									k = 0;
									line++;
								} else
									k++;

								this->unsetCharacterColorLocally(startPos, controller->getCrdt()->getSiteId());
							}
						} else {
							for (int i = 0; i < charsAdded; i++) {
								// for each char added
								qDebug() << cursor.position();
								cursor.setPosition(position + i);
								int line = cursor.blockNumber();
								int ch = cursor.positionInBlock();
								Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);
								// select char
								cursor.setPosition(position + i + 1, QTextCursor::KeepAnchor);
								QTextCharFormat charFormat = cursor.charFormat();

								if (i == charsAdded - 1) {
									controller->getCrdt()->localInsert(chars.at(i), charFormat, startPos, true);
								} else {
									controller->getCrdt()->localInsert(chars.at(i), charFormat, startPos, false);
								}

								this->unsetCharacterColorLocally(startPos, controller->getCrdt()->getSiteId());
							}
						}
					}
				}
			}

			restoreCursor();
			// onCursorPositionChanged();
		} else {
			qDebug() << " "; // new Line
			//qDebug() << "INVALID SIGNAL";
			//std::cout << "INVALID SIGNAL" << std::endl;
			if (this->startSelection != this->endSelection) {
				// text was selected... restore the selction
				restoreCursorSelection();
			} else {
				restoreCursor();
			}
		}
	} catch (...) {
		controller->reciveExternalErrorOrException();
		return;
	}

	this->updateOtherCursorPosition();

	connect(textEdit, &QTextEdit::cursorPositionChanged,
			this, &Editor::onCursorPositionChanged);

	qDebug() << this->textEdit->document()->blockCount();
}

void Editor::updateOtherCursorPosition() {
	QHash<QString, OtherCursor *>::const_iterator i;


	int width = this->textEdit->cursorRect().width();
	int height = this->textEdit->cursorRect().height();

	for (i = this->otherCursors.constBegin(); i != this->otherCursors.constEnd(); ++i) {
		qDebug() << i.key() << ':' << i.value()->getTextCursor().position();
		QRect coord = this->textEdit->cursorRect(i.value()->getTextCursor());
		i.value()->move(coord, width, height);
	}
}


void Editor::insertChar(char character, QTextCharFormat textCharFormat, Pos pos, QString siteId, Character c) {
	QTextDocument *doc = textEdit->document();
	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);

	disconnect(textEdit, &QTextEdit::cursorPositionChanged,
			   this, &Editor::onCursorPositionChanged);

	if (count > 0) {
		std::cout << "Editor " << c.getValue() << " Pos old: " << pos.getCh() << " " << pos.getLine() << std::endl;
		pos = controller->getCrdt()->findPosition(c);
		std::cout << "Editor " << c.getValue() << " Pos new: " << pos.getCh() << " " << pos.getLine() << std::endl;
	}
	controller->editor->pendingChar.pop_front();

	int oldCursorPos = textCursor.position();

	textCursor.movePosition(QTextCursor::Start);
	textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, pos.getLine());
	textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, pos.getCh());

	textCursor.insertText(QString::fromLatin1(&character, 1));

	QRect coord = this->textEdit->cursorRect(textCursor);
	int width = this->textEdit->cursorRect().width();

	qDebug() << "Cursor width: " << width;
	int height = this->textEdit->cursorRect().height();
	qDebug() << "Cursor height: " << height;

	this->otherCursors[siteId]->move(coord, width, height);

	this->otherCursors[siteId]->setOtherCursorPosition(textCursor.position());

	qDebug() << "Pos text cursor (after insert): " << textCursor.position();
	qDebug() << "Pos other text cursor (after insert): " << this->otherCursors[siteId]->getTextCursor().position();


	textCursor.setPosition(textCursor.position() - 1, QTextCursor::KeepAnchor);
	textCursor.mergeCharFormat(textCharFormat);
	textEdit->mergeCurrentCharFormat(textCharFormat);


	qDebug() << siteId;

	connect(doc, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	textCursor.setPosition(oldCursorPos);

	connect(textEdit, &QTextEdit::cursorPositionChanged,
			this, &Editor::onCursorPositionChanged);

	if (this->otherCursors[siteId]->isSelected1()) {
		this->setCharacterColorLocally(pos, siteId);
	} else {
		this->unsetCharacterColorLocally(pos, siteId);
	}
}

void Editor::changeStyle(Pos pos, const QTextCharFormat &textCharFormat, QString siteId) {
	QTextCursor tmpTextCursor(this->textEdit->textCursor());
	QTextCursor otherCursor;
	otherCursor.movePosition(QTextCursor::End);
	textEdit->setTextCursor(otherCursor);
	int oldCursorPos = textCursor.position();


	textCursor.movePosition(QTextCursor::Start);
	textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, pos.getLine());
	textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, pos.getCh());


	QTextDocument *doc = textEdit->document();
	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);

	QRect coord = this->textEdit->cursorRect(textCursor);
	int width = this->textEdit->cursorRect().width();

	qDebug() << "Cursor width: " << width;
	int height = this->textEdit->cursorRect().height();
	qDebug() << "Cursor height: " << height;

	this->otherCursors[siteId]->move(coord, width, height);

	this->otherCursors[siteId]->setOtherCursorPosition(textCursor.position());

	qDebug() << "Pos text cursor (after style change): " << textCursor.position();
	qDebug() << "Pos other text cursor (after style change): "
			 << this->otherCursors[siteId]->getTextCursor().position();

	textCursor.setPosition(textCursor.position() + 1, QTextCursor::KeepAnchor);
	textCursor.mergeCharFormat(textCharFormat);
	textEdit->mergeCurrentCharFormat(textCharFormat);
	mergeFormatOnWordOrSelection(textCharFormat);

	connect(doc, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	textEdit->setTextCursor(tmpTextCursor);
	textCursor.setPosition(oldCursorPos);
}


void Editor::deleteChar(Pos pos, QString sender) {

	qDebug() << "\nEditor.cpp - deleteChar():";
	qDebug() << "\t\tDelete char at:\n";
	qDebug() << "\t\t\tline:\t" << pos.getLine() << "\n\t\t\tch:\t" << pos.getCh();
	qDebug() << "\n\t\tInserted by:\t" << siteId;


	int oldCursorPos = textCursor.position();

	qDebug() << "\n\t\tInitial cursor position:\t" << oldCursorPos;

	textCursor.movePosition(QTextCursor::Start);
	textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, pos.getLine());
	textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, pos.getCh());


	QTextDocument *doc = textEdit->document();
	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);

	QChar deletedChar = textEdit->document()->characterAt(textCursor.position());

	textCursor.deleteChar();

	qDebug() << "\n\t\tDeleted char:\t" << deletedChar;

	QRect coord = this->textEdit->cursorRect(textCursor);
	int width = this->textEdit->cursorRect().width();
	int height = this->textEdit->cursorRect().height();


	this->otherCursors[sender]->move(coord, width, height);


	connect(doc, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	textCursor.setPosition(oldCursorPos);

	qDebug() << "\n\t\tFinal cursor position:\t" << textCursor.position();

	return;
}

void Editor::setFormat(CharFormat charFormat) {
	QTextCharFormat fmt = charFormat.toTextCharFormat();

	mergeFormatOnWordOrSelection(fmt);
}

void Editor::onCursorPositionChanged() {
	QTextCursor cursor = textEdit->textCursor();
	if (!cursor.hasSelection()) {
		int cursorPos = cursor.position();
		if (cursorPos == 0) {
			setFormat(
					CharFormat());
		} else if (cursorPos > 0) {
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
	this->updateAlignmentPushButton();
}

/**
 * This method allows to open a new file.
 */
void Editor::on_actionNew_File_triggered() {
	if (controller->getMessanger()->messagesIsEmpty() && !controller->getCrdt()->isWorking1() &&
		controller->getCrdt()->getNumJobs() == 0) {
		CreateFile *createFile = new CreateFile(controller->getGui());
		connect(createFile, SIGNAL(createFile(QString)), this->controller, SLOT(requestForFile(QString)));
		createFile->show();
	} else
		QMessageBox::information(controller->GUI, "Wait!", "The program is finishing processing the data!");
}

void Editor::on_actionShare_file_triggered() {
	SimpleCrypt crypto;                                                    /* ATTENZIONE: Stesso codice presente in getsharecode in showfile!!!*/
	crypto.setKey(0xf55f15758b7e0153); // Random generated key, same must be used server side!!!
	QString fileName; // complete filename username + "%_##$$$##_%" + filename (form 1)
	QString filenameForLabel; // just the filename given from the user (form 2)
	QString userName = controller->getUser()->getUsername(); //username

	//Try to split to see if filename is in form 1 or 2
	QStringList filenameFields = this->filename.split("%_##$$$##_%");

	if (filenameFields.size() == 1) {
		// Filename is the short filename form 2
		fileName = userName + "%_##$$$##_%" + filenameFields[0]; // get filename in form 1
		filenameForLabel = filenameFields[0]; //filename in form 2
	} else {
		fileName = userName + "%_##$$$##_%" + filenameFields[1]; // get filename in form 1
		filenameForLabel = filenameFields[1]; //filename in form 2
	}

	qDebug() << "Filename: " + fileName;
	qDebug() << "Filename label: " + filenameForLabel;
	qDebug() << "Username : " + userName;

	QString shareCode = crypto.encryptToString(userName + "%_##$$$##_%" + fileName);
	qDebug() << "Sharecode : " + shareCode;
	auto *shareFile = new ShareFile(this, filenameForLabel, shareCode);
	shareFile->show();
}

/**
 * This method permittes to open the showFile view.
 */
void Editor::on_actionOpen_triggered() {
	if (controller->getMessanger()->messagesIsEmpty() && !controller->getCrdt()->isWorking1() &&
		controller->getCrdt()->getNumJobs() == 0)
			emit showFinder();
	else
		QMessageBox::information(controller->GUI, "Wait!", "The program is finishing processing the data!");
}

void Editor::on_actionSave_as_PDF_triggered() {
	if (controller->getMessanger()->messagesIsEmpty() && !controller->getCrdt()->isWorking1() &&
		controller->getCrdt()->getNumJobs() == 0) {
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

		QMessageBox::information(controller->GUI, "PDF", "File Esportato");
	} else
		QMessageBox::information(controller->GUI, "Wait!", "The program is finishing processing the data!");
}

/**
 * This method allows to log out from application.
 */
void Editor::on_actionLogout_triggered() {
	qDebug() << controller->getCrdt()->getNumJobs();
	if (controller->getMessanger()->messagesIsEmpty() && !controller->getCrdt()->isWorking1() &&
		controller->getCrdt()->getNumJobs() == 0) {
		emit logout();
	} else
		QMessageBox::information(controller->GUI, "Wait!", "The program is finishing processing the data!");
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
	if (charsAdded == charsRemoved && position + charsAdded > textDocument->characterCount() - 1 &&
		textAdded == textRemoved) {
		// wrong signal when apply style in editor 1 (line 2) and then write something in editor2
		qDebug() << "WRONG SIGNAL 1";
		validSignal = false;
	}

	int currentDocumentSize = textDocument->characterCount() - 1;
	undo();
	int undoDocumentSize = textDocument->characterCount() - 1;
	redo();
	//qDebug() << "currentDocumentSize" << currentDocumentSize << " undoDocumentSize" <<undoDocumentSize;
	if (validSignal && charsAdded == charsRemoved &&
		currentDocumentSize != (undoDocumentSize + charsAdded - charsRemoved)) {
		// wrong signal when editor gets focus and something happen.
		validSignal = false;
	}

	/*if(validSignal && charsAdded == charsRemoved && (position+charsRemoved) > (textDocument->characterCount()-1)) {
		//qDebug() << "WRONG SIGNAL";
		// wrong signal when client add new line after it takes focus or when it move the cursor in the editor after the focus acquired
		validSignal = false;
	}*/

	QString test = textEdit->toPlainText().mid(position, charsAdded);
	if (validSignal && charsAdded == charsRemoved && test.isEmpty()) {
		// wrong signal when editor opens.
		//qDebug() << "WRONG SIGNAL 3";
		validSignal = false;
	}

	// check if text selected
	bool textSelected = false;
	int beginPos = textEdit->textCursor().selectionStart();
	int endPos = textEdit->textCursor().selectionEnd();
	int currentSize = textEdit->toPlainText().size();
	if (beginPos != endPos) {
		textSelected = true;
	}
	if (validSignal && textSelected && charsAdded == charsRemoved && currentSize != 0) {
		// this solve the bug when we select text (in multilines), when the textedit has not the fucus, and we delete them.
		//qDebug() << "WRONG SIGNAL 4";
		validSignal = false;
	}

	return validSignal;
}

/**
 * Handle the size of window programmaticaly mode.
 * @param event
 */
void Editor::resizeEvent(QResizeEvent *event) {
	ui->userListWidget->resize(149, textEdit->geometry().height() - 18 - 61);

	ui->userWidget->setGeometry(0, textEdit->geometry().height() - 18 - 61, ui->userWidget->width(),
								ui->userWidget->height());
	ui->offlineTextButton->setGeometry(0, textEdit->geometry().height() - 18 - 61 - 32, ui->offlineTextButton->width(),
                                       ui->offlineTextButton->height());
	this->updateOtherCursorPosition();
}

/**
 * This method allows to remove a specific user in the editor.
 * @param user
 */
void Editor::removeUser(QString user) {
	if (user == this->controller->getUser()->getUsername()) {
		showFinder();
		return;
	}
	qDebug() << "\nEditor.cpp - removeUsers():\n";
	qDebug() << "\t\tUSER REMOVED:\t" << user;

	try {
		if (!users.isEmpty())
			users.erase(std::remove_if(users.begin(), users.end(), [user](const QString &s) {
				return s == user;
			}));

		ui->userListWidget->clear();
		ui->userListWidget->addItems(users);
		this->otherCursors[user]->hide();
		otherCursors[user]->setIsSelected(false);
		this->hideUserText(user);
		this->otherCursors.remove(user);
		QString onlineUsers = "Online users: " + QString::number(users.size());
		ui->dockWidget->setTitleBarWidget(new QLabel(onlineUsers));
        if (offlineTextEnabled) {
            showOfflineUserText();
        }
	} catch (...) {
		controller->reciveExternalErrorOrException();
	}
}

/**
 * This method sets the user list in the editor.
 * @param users
 */
void Editor::setUsers(QStringList users) {
	qDebug() << "\nEditor.cpp - setUsers(): ";
	try {
		if (loadingFlag) {
			ui->loading->close();
			loadingFlag = false;
			QColor color(colors[colorIndex]);
			color.setAlpha(128); // opacity
			otherCursors.insert(this->controller->getUser()->getUsername(),
								new OtherCursor(this->controller->getUser()->getUsername(), this->textDocument, color,
												true,
												this->textEdit->viewport()));
		}

		bool isInsert = false;
		if (users.size() > 1) {
			ui->userListWidget->clear();
		} else {
			if (users.size() != 0)
				for (QString ur : this->users) {
					if (ur == users[0]) {
						isInsert = true;
					}
				}
		}
		if (users.size() != 0) {
			if (!isInsert) {
				this->users.append(users);
				ui->userListWidget->addItems(users);
				std::for_each(users.begin(), users.end(), [this](QString user) {
					QColor color(colors[colorIndex]);
					color.setAlpha(128); // opacity
					otherCursors.insert(user,
										new OtherCursor(user, this->textDocument, color, false,
														this->textEdit->viewport()));
					colorIndex++;
					if (colorIndex == 14) {
						colorIndex = 0;
					}

                    if (offlineTextEnabled) {
                        hideUserText(user);
                    }

				});
			}
		} else {
			ui->userListWidget->clear();
		}
		controller->stopLoadingPopup();
		QString onlineUsers = "Online users: " + QString::number(users.size());
		ui->dockWidget->setTitleBarWidget(new QLabel(onlineUsers));

	} catch (...) {
		// resest the status of application
		controller->reciveExternalErrorOrException();
		return;
	}
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

void Editor::replaceText(const std::vector<std::vector<Character>> initialText) {
	QTextDocument *doc = textEdit->document();

	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);

	textEdit->clear();
	int l = 0;
	for (auto &line : initialText) {
		int ch = 0;
		for (auto &character : line) {
			char c = character.getValue();
			this->textEdit->textCursor().insertText(QString::fromLatin1(&c, 1), character.getTextCharFormat());
			Pos pos{ch, l};

			connect(doc, &QTextDocument::contentsChange,
					this, &Editor::onTextChanged);

			if (otherCursors.contains(character.getSiteId()) &&
				character.getSiteId() != controller->getCrdt()->getSiteId() &&
				otherCursors[character.getSiteId()]->isSelected1()) {
				this->setCharacterColorLocally(pos, character.getSiteId());

			} else {

				this->unsetCharacterColorLocally(pos, character.getSiteId());
			}

			disconnect(doc, &QTextDocument::contentsChange,
					   this, &Editor::onTextChanged);
			ch++;
		}
		l++;
	}

	connect(doc, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	QTextCursor newCursor = textEdit->textCursor();
	newCursor.movePosition(QTextCursor::End);
	textEdit->setTextCursor(newCursor);
	qDebug() << "Alignment: " << textEdit->alignment();
}

void Editor::reset() {
	ui->userListWidget->clear();
	changeUser();
	ui->loading->show();
}

void Editor::showError() {
	QMessageBox::information(controller->getGui(), "Error", "Error!");
	controller->handleError();
}

/**
 * This method sets a filename in the editor and permittes to handle the share botton.
 * @param filename
 */
void Editor::setFilename(QString filename) {
	this->filename = filename;
	qDebug() << filename << controller->getUser()->getFileList()[filename];
	if (filename.contains("%_##$$$##_%") &&
		filename.split("%_##$$$##_%")[0] != controller->getUser()->getUsername()) {
		ui->mainToolBar->actions().at(2)->setVisible(false);
	}
}

/**
 * This method changes the user avatar and username in the regarding user section.
 */
void Editor::changeUser() {
	QPixmap pix = controller->getUser()->getAvatar();
	int w = ui->avatar->width();
	int h = ui->avatar->height();
	ui->avatar->setPixmap(pix.scaled(w, h, Qt::KeepAspectRatio));
	ui->username->setText(controller->getUser()->getUsername());
}

void Editor::updateAlignmentPushButton() {
	Qt::Alignment alignment = this->textEdit->textCursor().blockFormat().alignment();

	switch (alignment) {
		case Qt::AlignHCenter:
			this->actionAlignCenter->setChecked(true);
			break;
		case Qt::AlignRight | Qt::AlignAbsolute:
			this->actionAlignRight->setChecked(true);
			break;
		case Qt::AlignJustify:
			this->actionAlignJustify->setChecked(true);
			break;
		default:
			this->actionAlignLeft->setChecked(true);
			break;
	}
}


void Editor::onListUsersItemClicked(QListWidgetItem *item) {
	QString user = item->text();
	qDebug() << "Utente Cliccato" << user;
	bool oldSelectedValue = this->otherCursors[user]->isSelected1();
	this->otherCursors[user]->setIsSelected(!oldSelectedValue);
	if (!oldSelectedValue)
		this->showUserText(user);
	else
		this->hideUserText(user);
}

void Editor::textCopy() {
	textEdit->copy();
}

void Editor::textCut() {
	textEdit->cut();
}

void Editor::textPaste() {
	textEdit->paste();
}

void Editor::setCharacterColorLocally(Pos pos, QString user) {


	int oldCursorPos = textCursor.position();

	QTextCursor tmpTextCursor(this->textEdit->textCursor());
	QTextCursor otherCursor;
	otherCursor.movePosition(QTextCursor::End);
	textEdit->setTextCursor(otherCursor);


	textCursor.movePosition(QTextCursor::Start);
	textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, pos.getLine());
	textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, pos.getCh());


	QTextDocument *doc = textEdit->document();
	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);


	QTextCharFormat textCharFormat = textCursor.charFormat();
	QColor color;

	if (this->otherCursors.contains(user)) {
		color = this->otherCursors[user]->getColor();
	}else {
	    color = Qt::gray;
	}

	textCharFormat.setBackground(color);

	textCursor.setPosition(textCursor.position() + 1, QTextCursor::KeepAnchor);
	textCursor.mergeCharFormat(textCharFormat);
	//textEdit->mergeCurrentCharFormat(textCharFormat);

	connect(doc, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	textEdit->setTextCursor(tmpTextCursor);
	textCursor.setPosition(oldCursorPos);
	auto cursor = this->textEdit->textCursor();
}

void Editor::unsetCharacterColorLocally(Pos pos, QString user) {
	int oldCursorPos = textCursor.position();

	QTextCursor tmpTextCursor(this->textEdit->textCursor());
	QTextCursor otherCursor;
	otherCursor.movePosition(QTextCursor::End);
	textEdit->setTextCursor(otherCursor);


	textCursor.movePosition(QTextCursor::Start);
	textCursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, pos.getLine());
	textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, pos.getCh());


	QTextDocument *doc = textEdit->document();
	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);


	QTextCharFormat textCharFormat = textCursor.charFormat();
	/*QColor color;

	if( this->otherCursors.contains(user)){
		color=this->otherCursors[user]->getColor();
	}
*/
	textCharFormat.setBackground(Qt::white);

	textCursor.setPosition(textCursor.position() + 1, QTextCursor::KeepAnchor);
	textCursor.mergeCharFormat(textCharFormat);
	//textEdit->mergeCurrentCharFormat(textCharFormat);

	connect(doc, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	textEdit->setTextCursor(tmpTextCursor);
	textCursor.setPosition(oldCursorPos);
	auto cursor = this->textEdit->textCursor();


}


void Editor::showUserText(QString &user) {
	auto structure = this->controller->getCrdt()->getStructure();

	int r = 0;
	for (auto &row : structure) {
		int c = 0;
		for (auto &ch : row) {
			if (ch.getSiteId() == user) {
				Pos pos(c, r);
				this->setCharacterColorLocally(pos, user);
			}
			c++;
		}
		r++;
	}
}

void Editor::hideUserText(QString &user) {
	auto structure = this->controller->getCrdt()->getStructure();

	int r = 0;
	for (auto &row : structure) {
		int c = 0;
		for (auto &ch : row) {
			if (ch.getSiteId() == user) {
				Pos pos(c, r);
				this->unsetCharacterColorLocally(pos, user);
			}
			c++;
		}
		r++;
	}
}

void Editor::showOfflineUserText() {
    auto structure = this->controller->getCrdt()->getStructure();

    int r = 0;
    for (auto &row : structure) {
        int c = 0;
        for (auto &ch : row) {
            if (ch.getSiteId() != controller->getCrdt()->getSiteId() && !otherCursors.contains(ch.getSiteId())) {
                Pos pos(c, r);
                this->setCharacterColorLocally(pos, ch.getSiteId());
            }
            c++;
        }
        r++;
    }
}

void Editor::hideOfflineUserText() {
    auto structure = this->controller->getCrdt()->getStructure();

    int r = 0;
    for (auto &row : structure) {
        int c = 0;
        for (auto &ch : row) {
            if (ch.getSiteId() != controller->getCrdt()->getSiteId() && !otherCursors.contains(ch.getSiteId())) {
                Pos pos(c, r);
                this->unsetCharacterColorLocally(pos, ch.getSiteId());
            }
            c++;
        }
        r++;
    }
}


void Editor::handleOfflineText() {
    qDebug() << "handleOfflineText";
    offlineTextEnabled = !offlineTextEnabled;
    if (offlineTextEnabled) {
        showOfflineUserText();
        ui->offlineTextButton->setStyleSheet(QString("QPushButton{\nbackground-color: white;\nborder: 1px solid gray;\ncolor : gray;}"));
    }else{
        hideOfflineUserText();
        ui->offlineTextButton->setStyleSheet(QString("QPushButton{  \nbackground-color: gray;  \nborder: none;\ncolor: white;\ntext-align: center;\nmargin: 4px 2px;\nopacity: 0.6;\ntext-decoration: none;\n}\nQPushButton:hover{\nbackground-color: white;\nborder: 1px solid gray;\ncolor : gray;}"));

    }
}