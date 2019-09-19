#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QDialog>

namespace Ui {
class ShareFile;
}

class ShareFile : public QDialog
{
    Q_OBJECT

public:
    explicit ShareFile(QWidget *parent = nullptr, QString filename = "", QString shareCode = "");
    ~ShareFile();

public slots:
	void copyToClipboard();

private:
    Ui::ShareFile *ui;
    QString filename;
};

#endif // SHAREFILE_H
