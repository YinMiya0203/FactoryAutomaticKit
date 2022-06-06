#pragma once

#include <QDialog>
#include "ui_LoginDlg.h"

class LoginDlg : public QDialog
{
	Q_OBJECT

public:
	LoginDlg(QWidget *parent = Q_NULLPTR);
	~LoginDlg();
	bool eventFilter(QObject* watched, QEvent* event);
private slots:

    void on_pushButton_clicked();

private:
	Ui::LoginDlg ui;
};
