#include <QtGui/QClipboard>
#include "sharefile.h"
#include "ui_sharefile.h"

ShareFile::ShareFile(QWidget *parent, QString filename, QString shareCode) :
    QDialog(parent), filename(filename),
    ui(new Ui::ShareFile)
{
    ui->setupUi(this);
    ui->title->setText("Share " + filename);
    ui->label->setText(shareCode);
    ui->copyIcon->setPixmap(QPixmap(":/rec/img/copy.png"));

    connect(ui->copyIcon, SIGNAL(clicked()), this , SLOT(copyToClipboard()));
}

void ShareFile::copyToClipboard() {
	QClipboard *clipboard = QApplication::clipboard();
	QString stringToCopy = ui->label->text();
	clipboard->setText(stringToCopy,QClipboard::Clipboard);
}

ShareFile::~ShareFile()
{
    delete ui;
}