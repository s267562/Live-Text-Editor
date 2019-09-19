#include <QMessageBox>
#include "customwidget.h"
#include "ui_customwidget.h"
#include "sharefile.h"

CustomWidget::CustomWidget(QWidget *parent, QString filename, bool owned, QString username) :
		QWidget(parent), filename(filename),
		ui(new Ui::CustomWidget) {
	ui->setupUi(this);
	if (owned) {
		ui->share->setPixmap(QPixmap(":/rec/img/share.png"));
		fileShareCode = QString(username + "%_##$$$##_%" + filename).toLatin1().toHex();
	}
	else {
		ui->network->setPixmap(QPixmap(":/rec/img/network.png"));
		ui->share->setEnabled(false);
	}
	ui->fileName->setText(filename);

	connect(ui->share, SIGNAL(clicked()), this, SLOT(pushSharedButton()));
}

CustomWidget::~CustomWidget() {
	delete ui;
}

void CustomWidget::pushSharedButton() {
	ShareFile *shareFile = new ShareFile(this, filename, fileShareCode);
	shareFile->show();
}
