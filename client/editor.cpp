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

Editor::Editor(QString siteId, QWidget *parent) : textEdit(new QTextEdit(this)), textDocument(textEdit->document()),
												  siteId(siteId), QMainWindow(parent), ui(new Ui::Editor) {
	ui->setupUi(this);
	setWindowTitle(QCoreApplication::applicationName());
	setCentralWidget(textEdit);

	ui->dockWidget->setTitleBarWidget(new QLabel("Online users"));

	ui->userListWidget->resize(this->geometry().width(), this->geometry().height());

	QPixmap pix;
	pix.load("/Users/andrea/Documents/sfondi/preview.jpeg");
	// TODO: from QByteArray to QPixMap

	ui->actionAvatar->setIcon(QIcon(pix));
	ui->actionAvatar->setIconVisibleInMenu(true);

	this->textCursor = textEdit->textCursor();

	// Controller
	connect(textDocument, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	//connect(ui->actionNew_File, &QAction::triggered, this, &Editor::on_actionNew_file_triggered);
	/*connect(ui->actionOpen, &QAction::triggered, this, &Editor::on_actionOpen_triggered);
	connect(ui->actionShare_file, &QAction::triggered, this, &Editor::on_actionShare_file_triggered);
	connect(ui->actionSave_as_PDF, &QAction::triggered, this, &Editor::on_actionSave_as_PDF_triggered);
	connect(ui->actionLogout, &QAction::triggered, this, &Editor::on_actionLogout_triggered);*/
}

void Editor::setController(Controller *controller) {
	Editor::controller = controller;
}

void Editor::onTextChanged(int position, int charsRemoved, int charsAdded) {
	//qDebug() << "editor.cpp - onTextChanged()     position: " << position << " chars added: " << charsAdded << " chars removed: " << charsRemoved;

	if (validSignal(position, charsAdded, charsRemoved)) {
		//qDebug() << "VALID SIGNAL";
		//std::cout << "VALID SIGNAL" << std::endl;

		int currentSize = textEdit->toPlainText().size();
		if (position == 0 && currentSize != charsAdded && charsAdded > 0 && charsRemoved > 0) {
			//qDebug() << "editor.cpp - onTextChanged()     Paste in first position";
			// esempi:
			// ciaoiao(ciao) --> aggiunti 7, rimossi 4 (ultimi 4)
			// ciaociao(ciao) --> aggiunti 8, rimossi 4 (ultimi 4)
			// ciaoao(ciao) --> aggiunti 6, rimossi 4 (ultimi 4)
			// --> cancello dal modello ciò che c'era prima (ciao) e aggiungo il resto.

			// paste operation... (decrement added)
			charsAdded--;
			charsRemoved--;
			int line, ch;

			if (charsRemoved) {
				// TODO reset model (client-server)
				// this->controller->resetModel();

				// TODO to remove all these lines after reset model implemented...
				// get endPos
				undo();
				textCursor.movePosition(QTextCursor::End);
				line = textCursor.blockNumber();
				ch = textCursor.positionInBlock();
				Pos endPos{ch, line}; // Pos(int ch, int line);
				redo();
				std::cout << endPos << std::endl;
				// delete everithing before paste...
				this->controller->localDelete(Pos{0, 0}, endPos);
			}


			QString chars = textEdit->toPlainText().mid(position, charsAdded);
			// get start position
			textCursor.setPosition(position);
			line = textCursor.blockNumber();
			ch = textCursor.positionInBlock();
			Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);
			this->controller->localInsert(chars, startPos);

		} else {

			if (charsAdded) {
				QString chars = textEdit->toPlainText().mid(position, charsAdded);

				// get start position
				textCursor.setPosition(position);
				int line = textCursor.blockNumber();
				int ch = textCursor.positionInBlock();
				Pos startPos{ch, line}; // Pos(int ch, int line, const std::string);

				this->controller->localInsert(chars, startPos);
			}

			if (charsRemoved) {
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
				//qDebug() << "startPos:" << startPos.getLine() << startPos.getCh();
				//qDebug() << "endPos:" << endPos.getLine() << endPos.getCh();
				this->controller->localDelete(startPos, endPos);
			}
		}
	} else {
		//qDebug() << "INVALID SIGNAL";
		//std::cout << "INVALID SIGNAL" << std::endl;
	}

	/*
	// check if text selected
	bool textSelected = false;
	if(textCursor.selectionStart() != textCursor.selectionEnd()) {
		textSelected = true;
	}
	if(beginPos != endPos) {
		textSelected = true;
	}

	 ...

	// reset cursor status
	if(textSelected) {
		cursor.setPosition(beginPos);
		cursor.setPosition(endPos, QTextCursor::KeepAnchor);
	} else {
		cursor.setPosition(cursorPos);
	}
	textEdit->setTextCursor(cursor);
	*/
}

void Editor::insertChar(char character, Pos pos) {
	int oldCursorPos = textCursor.position();

	textCursor.movePosition(QTextCursor::Start);
	textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
	textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());

	QTextDocument *doc = textEdit->document();
	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);

	textCursor.insertText(QString{character});

	connect(doc, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	textCursor.setPosition(oldCursorPos);
}

void Editor::deleteChar(Pos pos) {
	int oldCursorPos = textCursor.position();

	textCursor.movePosition(QTextCursor::Start);
	textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, pos.getLine());
	textCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, pos.getCh());

	QTextDocument *doc = textEdit->document();
	disconnect(doc, &QTextDocument::contentsChange,
			   this, &Editor::onTextChanged);

	textCursor.deleteChar();

	connect(doc, &QTextDocument::contentsChange,
			this, &Editor::onTextChanged);

	textCursor.setPosition(oldCursorPos);
}

void Editor::on_actionNew_File_triggered() {
	QMessageBox::information(this, "File", "File!");
}

void Editor::on_actionShare_file_triggered() {
	QMessageBox::information(this, "Share", "Share!");
}

void Editor::on_actionOpen_triggered() {
	QMessageBox::information(this, "Open", "Open!");
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
	QMessageBox::information(this, "Logout", "Logout!");
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

	QString test = textEdit->toPlainText().mid(position, charsAdded);
	if (charsAdded == charsRemoved && test.isEmpty()) {
		// wrong signal when edito opens.
		validSignal = false;
	}

	textCursor.movePosition(QTextCursor::StartOfLine);
	int positionFirstLineChar = textCursor.position(); // the position of the first char in the current line .
	int currentSize = textEdit->toPlainText().size();
	if (validSignal && charsAdded == charsRemoved && position == positionFirstLineChar && currentSize != 0) {
		// wrong signal when cursor move after the editor get focuses.
		// std::cout << "current size: " << currentSize << std::endl;
		validSignal = false;
	}

	// check if text selected
	bool textSelected = false;
	int beginPos = textEdit->textCursor().selectionStart();
	int endPos = textEdit->textCursor().selectionEnd();
	if (beginPos != endPos) {
		textSelected = true;
	}
	if (validSignal && textSelected && charsAdded == charsRemoved && currentSize != 0) {
		// this solve the bug when we select text (in multilines), when the textedit has not the fucus, and we delete them.
		validSignal = false;
	}

	return validSignal;
}

void Editor::resizeEvent(QResizeEvent *event) {
	ui->userListWidget->resize(textEdit->geometry().width(), textEdit->geometry().height() - 18);
}

void Editor::removeUser(QString user) {
	users.erase(std::remove_if(users.begin(), users.end(), [user](const QString &s) {
		return s == user;
	}));

	ui->userListWidget->clear();
	ui->userListWidget->addItems(users);
}

void Editor::setUsers(QStringList users) {
	this->users = users;
	ui->userListWidget->addItems(users);
}