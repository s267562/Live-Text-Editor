#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QTextCharFormat;
QT_END_NAMESPACE

class TextEdit : public QMainWindow {
    Q_OBJECT

public:
    TextEdit(QWidget *parent = nullptr);

    bool load(const QString &f);

protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void fileExportPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textAlign(QAction *a);
    void textColor();

private:
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    bool maybeSave();
    void setCurrentFileName(const QString &fileName);

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void colorChanged(const QColor &c);

    QAction *actionSave;
    QAction *actionUndo;
    QAction *actionRedo;
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionTextColor;

    QString fileName;
    QTextEdit *textEdit;

};

#endif // TEXTEDIT_H