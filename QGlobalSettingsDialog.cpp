#include "QGlobalSettingsDialog.h"
#include "GlobalConfig.h"
#include "GlobalSettings.h"

#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QDesktopWidget>
QGlobalSettingsDialog::QGlobalSettingsDialog(bool islogin)
{
	setWindowFlags(Qt::Dialog);
	ptopVBoxLayout = new QVBoxLayout(this);
	ptopwidght = new QWidget(this);
	ptopwidght->setObjectName("ptopwidght");
	ptopVBoxLayout->addWidget(ptopwidght);
	if(islogin) {
		auto passowdLayout = new QVBoxLayout;
		ptopwidght->setLayout(passowdLayout);

		auto userlabel = new QLabel;
		userlabel->setText("User:");
		passowdLayout->addWidget(userlabel);
		//tmpwdigetcontain.push_back(userlabel);

		auto userline = new QLineEdit;
		passowdLayout->addWidget(userline);
		//tmpwdigetcontain.push_back(userline);
		userline->setObjectName(QString("userline"));
		userline->setInputMethodHints(Qt::InputMethodHint::ImhPreferLowercase);
		userline->setText("root");

		auto passwdlabel = new QLabel;
		passwdlabel->setText("Passwd:");
		passowdLayout->addWidget(passwdlabel);
		//tmpwdigetcontain.push_back(passwdlabel);

		auto passwdline = new QLineEdit;
		passwdline->setEchoMode(QLineEdit::EchoMode::Password);
		//tmpwdigetcontain.push_back(passwdline);
		passwdline->setObjectName(QString("passwdline"));
		passowdLayout->addWidget(passwdline);

		auto okpb = new QPushButton;
		okpb->setText("OK");
		passowdLayout->addWidget(okpb);
		connect(okpb, &QPushButton::clicked, this, &QGlobalSettingsDialog::on_okpb_clicked);

		auto cancelpb = new QPushButton;
		cancelpb->setText("Cancel");
		passowdLayout->addWidget(cancelpb);
		connect(cancelpb, &QPushButton::clicked, this, [this]() {
			reject();
			});
	}
	else {
		setupsettingview();
	}
	
	setLayout(ptopVBoxLayout);
}
void QGlobalSettingsDialog::windowCenter()
{
	QRect screen = QDesktopWidget().screenGeometry();
	this->resize(QSize(screen.width()/2, screen.height()/4));
	QRect size = this->geometry();
	
	this->move((screen.width() - size.width()) / 2, (screen.height() - size.height()) / 2);
	//this->showMaximized();
}
int32_t QGlobalSettingsDialog::DeleteView(QWidget* dev_widget)
{
	auto old_laylout = dev_widget->layout();
	if (old_laylout != nullptr) {
		QLayoutItem* child;
		while ((child = old_laylout->takeAt(0)) != 0)
		{
			//setParent为NULL，防止删除之后界面不消失
			if (child->widget())
			{
				child->widget()->setParent(NULL);
			}

			delete child;

		}
		delete dev_widget->layout();
	}
	return 0;
}
void QGlobalSettingsDialog::setupsettingview()
{
	auto settingsLayout = new QVBoxLayout;
	ptopwidght->setLayout(settingsLayout);
	windowCenter();
	//log location
	{
		auto itemwidget = new QWidget;
		auto itemlayout = new QHBoxLayout;
		settingsLayout->addWidget(itemwidget);
		itemwidget->setLayout(itemlayout);

		auto textlabel = new QLabel;
		textlabel->setText(QStringLiteral("测试结果: "));
		itemlayout->addWidget(textlabel);

		auto pathline = new QLineEdit;
		pathline->setText(GLOBALSETTINGSINSTANCE->GetTestCaseResultDirLocation());
		itemlayout->addWidget(pathline);
		if (!GLOBALSETTINGSINSTANCE->isUserRoot()) {
			pathline->setReadOnly(true);
		}
		pathline->setObjectName("linelabel_tcresult");
		auto selectpb = new QPushButton;
		itemlayout->addWidget(selectpb);
		selectpb->setText(QStringLiteral("选择"));

		connect(selectpb, &QPushButton::clicked, this, [this, pathline]() {
			QString dirpath = QFileDialog::getExistingDirectory(this, QStringLiteral("选择目录"), GLOBALSETTINGSINSTANCE->GetTestCaseResultDirLocation(), QFileDialog::ShowDirsOnly);
			if (dirpath.size() > 0 && QDir(dirpath).exists()) {
				GLOBALSETTINGSINSTANCE->SetTestCaseResultDirLocation(dirpath);
				pathline->setText(GLOBALSETTINGSINSTANCE->GetTestCaseResultDirLocation());
			}
			});
	
	}
	//result location
	{
		auto itemwidget = new QWidget;
		auto itemlayout = new QHBoxLayout;
		settingsLayout->addWidget(itemwidget);
		itemwidget->setLayout(itemlayout);

		auto textlabel = new QLabel;
		textlabel->setText(QStringLiteral("日志路径: "));
		itemlayout->addWidget(textlabel);

		auto pathline = new QLineEdit;
		if (!GLOBALSETTINGSINSTANCE->isUserRoot()) {
			pathline->setReadOnly(true);
		}
		pathline->setObjectName("linelabel_log");
		pathline->setText(GLOBALSETTINGSINSTANCE->GetLogDirLocation());
		itemlayout->addWidget(pathline);
		auto selectpb = new QPushButton;
		selectpb->setText(QStringLiteral("选择"));
	
		itemlayout->addWidget(selectpb);
		connect(selectpb, &QPushButton::clicked, this, [this, pathline]() {
			QString dirpath = QFileDialog::getExistingDirectory(this, QStringLiteral("选择目录"), GLOBALSETTINGSINSTANCE->GetTestCaseResultDirLocation(), QFileDialog::ShowDirsOnly);
			if (dirpath.size() > 0 && QDir(dirpath).exists()) {
				GLOBALSETTINGSINSTANCE->SetLogDirLocation(dirpath);
				pathline->setText(GLOBALSETTINGSINSTANCE->GetLogDirLocation());
			}
			});
	}
	//ok
	{
		auto itemwidget = new QWidget;
		auto itemlayout = new QHBoxLayout;
		settingsLayout->addWidget(itemwidget);
		itemwidget->setLayout(itemlayout);

		auto okpb = new QPushButton;
		okpb->setText(QStringLiteral("确定"));
		itemlayout->addWidget(okpb);
		connect(okpb, &QPushButton::clicked, this, [this]() {
			qDebug(" ");
			GLOBALSETTINGSINSTANCE->SaveSettings();
			accept();
			});

		auto cancelpb = new QPushButton;
		cancelpb->setText(QStringLiteral("取消"));
		itemlayout->addWidget(cancelpb);
		connect(cancelpb, &QPushButton::clicked, this, [this]() {
			reject();
			});
	}
	//cancel
}

void QGlobalSettingsDialog::on_okpb_clicked(int)
{
#if 0
	auto widgets = this->findChildren<QWidget*>();
	qDebug("widgets %d", widgets.size());
	int index = 0;
	foreach(auto item, widgets) {
		qDebug("widgets %d [%s]", index, item->objectName().toStdString().c_str());
		index++;
	}
#endif
	QPushButton* rb = qobject_cast<QPushButton*>(sender());
	auto userline = this->findChild<QLineEdit*>(QString("userline"));
	auto passwdline = this->findChild<QLineEdit*>(QString("passwdline"));
	if (userline!=nullptr) {
		if (GLOBALSETTINGSINSTANCE->SetUserRoot(userline->text(), passwdline->text())==0 ) {
			//new view
			DeleteView(ptopwidght);
			setupsettingview();
		}
		else {
			QMessageBox::warning(this,"Waring","User Name or Passwd incorrect", QMessageBox::Yes);
		}
	}
	else {
		accept();
		
	}
}
