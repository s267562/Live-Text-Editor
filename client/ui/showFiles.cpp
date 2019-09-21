#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include "showFiles.h"
#include "ui_showFiles.h"
#include "../Networking/Messanger.h"
#include "customwidget.h"
#include "editaccount.h"
#include "../../server/SimpleCrypt/SimpleCrypt.h"

ShowFiles::ShowFiles(QWidget *parent, Controller *controller) :
		QMainWindow(parent),
		controller(controller),
		ui(new Ui::ShowFiles) {
	ui->setupUi(this);
	//connect(this,SIGNAL(newFile(QString)),this->parent(), SLOT(requestForFile(QString)));
	ui->filename->close();
	ui->pushButton_newFile->close();

	ui->newFile->setPixmap(QPixmap(":/rec/img/new-file.png"));
	ui->logout->setPixmap(QPixmap(":/rec/img/logout.png"));
	ui->avatar->setPixmap(QPixmap(":/rec/img/user.png"));
	connect(ui->newFile, SIGNAL(clicked()), this, SLOT(on_actionNew_File_triggered()));
	connect(ui->logout, SIGNAL(clicked()), this, SLOT(on_actionLogout_triggered()));
	connect(ui->avatar, SIGNAL(clicked()), this, SLOT(editAccount()));

	QGraphicsDropShadowEffect *m_shadowEffect = new QGraphicsDropShadowEffect(this);
	m_shadowEffect->setColor(QColor(0, 0, 0, 255 * 0.1));
	m_shadowEffect->setXOffset(0);
	m_shadowEffect->setYOffset(4);
	m_shadowEffect->setBlurRadius(12);
// hide shadow
	m_shadowEffect->setEnabled(true);
	ui->customToolbar->setGraphicsEffect(m_shadowEffect);
}

ShowFiles::~ShowFiles() {
	delete ui;
}

void ShowFiles::on_listWidget_itemDoubleClicked(QListWidgetItem *item) {
	QString filename = item->text();

	emit newFile(filename);
}

void ShowFiles::addFiles(std::map<QString, bool> l) {
	this->ui->listWidget->clear();

	for (std::pair<QString, bool> filename : l) {
		QString shareCode = "ERROR";
		// If user is owner for that file create a sharecode
		if (filename.second) {
			QString username = controller->getUser()->getUsername();
			shareCode = getShareCode(username, filename.first);
		}

		CustomWidget *myItem = new CustomWidget(this, filename.first, filename.second, shareCode);
		QListWidgetItem *item = new QListWidgetItem(filename.first);
		item->setSizeHint(QSize(0, 40));
		this->ui->listWidget->addItem(item);
		this->ui->listWidget->setItemWidget(item, myItem);
	}
}

void ShowFiles::on_pushButton_newFile_clicked() {
	QString filename = this->ui->filename->text();
	emit newFile(filename);
}

void ShowFiles::on_actionNew_File_triggered() {
	//QMessageBox::information(this, "File", "File!");
	if (!newFileShown) {
		ui->filename->show();
		ui->pushButton_newFile->show();
		ui->listWidget->move(10, 102);
		newFileShown = true;
	} else {
		ui->filename->close();
		ui->pushButton_newFile->close();
		ui->listWidget->move(10, 70);
		newFileShown = false;
	}

}

void ShowFiles::on_actionLogout_triggered() {
	QMessageBox::information(this, "Logout", "Logout!");
	emit logout();
}

void ShowFiles::resizeEvent(QResizeEvent *event) {
	ui->customToolbar->setGeometry(0, 0, width(), 60);
}


void ShowFiles::showError() {
	QMessageBox::information(this, "Error", "Error for this request!");
}

void ShowFiles::editAccount() {
	EditAccount *editAccount = new EditAccount(this, controller->getUser());
	connect(editAccount, SIGNAL(edit(QString, QString, QString, QByteArray)), controller,
			SLOT(sendEditAccount(QString, QString, QString, QByteArray)));
	editAccount->show();
}

/**
 * Generate the unique share code for a given file.
 * separator used: "%_##$$$##_%"
 * crypto key: 0xf55f15758b7e0153
 * @param username : owner of the file to share
 * @param filename : name of the file to share
 * @return
 */
QString ShowFiles::getShareCode(const QString &username, const QString &filename) {

	SimpleCrypt crypto;
	crypto.setKey(0xf55f15758b7e0153); // Random generated key, same must be used server side!!!

	QString shareCode = crypto.encryptToString(username + "%_##$$$##_%" + filename);
	return shareCode;
}
