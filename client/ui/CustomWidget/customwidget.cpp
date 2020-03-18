#include <QMessageBox>
#include <FileInformation/fileinformation.h>
#include "customwidget.h"
#include "ui_customwidget.h"
#include "ShareFile/sharefile.h"

CustomWidget::CustomWidget(QWidget *parent, QString filename, QString owner, bool owned, QString sharecode, Controller *controller) :
		QWidget(parent), filename(filename), fileShareCode(sharecode),
		ui(new Ui::CustomWidget) {
	ui->setupUi(this);
    ui->settings->setPixmap(QPixmap(":/rec/img/menu_2.png"));
	if (owner == controller->getUser()->getUsername()) {
        ui->owner->setText("You");
		ui->share->setPixmap(QPixmap(":/rec/img/share.png"));
        ui->network->setPixmap(QPixmap(":/rec/img/user singolo.png"));
	}
	else {
		ui->network->setPixmap(QPixmap(":/rec/img/network.png"));
		ui->share->setEnabled(false);
        ui->owner->setText(owner);
    }
	ui->fileName->setText(filename);
	this->owner = owner;
	this->controller = controller;

	connect(ui->share, SIGNAL(clicked()), this, SLOT(pushSharedButton()));
	connect(ui->settings, SIGNAL(clicked()), this, SLOT(pushSettingsButton()));
}

CustomWidget::~CustomWidget() {
	delete ui;
}

void CustomWidget::pushSharedButton() {
	ShareFile *shareFile = new ShareFile(this, filename, fileShareCode);
	shareFile->show();
}

void CustomWidget::pushSettingsButton() {
    controller->requestForUsernameList(owner + "%_##$$$##_%" + ui->fileName->text(),this);
}

void CustomWidget::createFileInformation(QStringList usernameList){
    FileInformation *fileInformation = new FileInformation(this, ui->fileName->text(), usernameList, controller, owner == controller->getUser()->getUsername());
    fileInformation->show();
}