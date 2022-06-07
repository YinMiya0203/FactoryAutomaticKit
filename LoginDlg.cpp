#include "LoginDlg.h"
#include "TestCase.h"
#include "ui_GlobalStr.h"
#include "QGlobalSettingsDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>
#include "GlobalSettings.h"
LoginDlg::LoginDlg(QWidget *parent)
	: QDialog(parent)
{
	//Q_INIT_RESOURCE(FactoryAutoTest);

	ui.setupUi(this);
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/res/image/machine.ico"), QSize(), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(icon);
	this->installEventFilter(this);
	this->setWindowTitle("Launch");
}
bool LoginDlg::eventFilter(QObject* watched, QEvent* event)
{
	//qDebug("type %d", event->type());
	if (event->type() == QEvent::MouseButtonPress) {
		//qDebug("type %d", event->type());
	}
	if (event->type() == QEvent::KeyPress) {
		//qDebug("type %d", event->type());
		auto e = dynamic_cast <QKeyEvent*>(event);
		if (e != nullptr) {
			if (e->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) && e->key() == Qt::Key_R) {

				qDebug("root window ");
				auto win = new QGlobalSettingsDialog(true);
				win->exec();
				delete win;
			}
		}
	}
	return QDialog::eventFilter(watched, event);
}
LoginDlg::~LoginDlg()
{
}


void LoginDlg::on_pushButton_clicked()
{
	QString op = ui.lineEdit_op->text();
	if( !GLOBALSETTINGSINSTANCE->isUserRoot() && op.size()==0 ) 
	{
		QMessageBox::warning(this, WARN_STR, tr("请输入测试员姓名"), QMessageBox::Yes);
		return;
	}
	if (!GLOBALSETTINGSINSTANCE->IsSettingParamLoad()) {
		QMessageBox::critical(this, ERROR_STR, tr("参数加载失败,请先确认User权限"), QMessageBox::Yes);
		return;
	}
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开测试用例"), "." , tr("INI File(*.ini)"));
	if (fileName.isEmpty()) {
		return; 	
	}
	if (!TestcaseBase::get_instance()->LoadFile(fileName)) {
		QString msg = fileName.append(TESTCASEFILE_UNABLE_LOAD);
		QMessageBox::critical(this, ERROR_STR,msg, QMessageBox::Yes);
	}
	else {
		qInfo("Open %s", fileName.toStdString().c_str());
		GLOBALSETTINGSINSTANCE->SetCurrentOp(op);
		accept();
	}

}
