#include "QDeviceDriverRunDialog.h"
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#define DEFAULT_DEV "0"
QStringList VisaDriverIoctrlList() {
	QStringList mlist;
	mlist << "Io"
		<< "IoRead"
		<< "IoWrite"
		<< "ReadSystemError"
		<< "ReadSystemErrorCount"
		<< "ReadIdentification"
		<< "OutputState"
		<< "SourceVoltage"
		<< "SourceCurrentLimit"
		<< "ReadQuery";
		return mlist;
}
QDeviceDriverRunDialog::QDeviceDriverRunDialog()
{
	setWindowFlags(Qt::Dialog);

	{

		int nWidth = GetSystemMetrics(SM_CXSCREEN);
		int nHeight = GetSystemMetrics(SM_CYSCREEN);
		setGeometry(nWidth / 2 - (nWidth/5), nHeight / 2 - (nHeight / 5), nWidth / 2, nHeight / 2);
	}
	ptopVBoxLayout = new QVBoxLayout(this);
	ptopVBoxLayout->setObjectName("ptopVBoxLayout");
	
	auto quitpb = new QPushButton;
	ptopVBoxLayout->addWidget(quitpb);
	quitpb->setText(QStringLiteral("退出"));
	connect(quitpb, &QPushButton::clicked, this, [this]() {
		this->accept();
		});
	auto mcomboBox = new QComboBox;
	ptopVBoxLayout->addWidget(mcomboBox);
	mcomboBox->addItems(VisaDriverIoctrlList());
	//mcomboBox->setCurrentIndex(int(VisaDriverIoctrl::ReadSystemErrorCount));
	//void currentlndexChanged(int index)
	//	void currentlndexChanged(const QString & text)
	//connect(mcomboBox, SIGNAL(currentlndexChanged(int)), this, SLOT(mcomboBox_currentlndexChanged(int)));
	connect(mcomboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QDeviceDriverRunDialog::mcomboBox_currentlndexChanged);
	//connect(mcomboBox, SIGNAL(currentlndexChanged(const QString&)), this, SLOT(mcomboBox_currentlndexChanged(const QString&)));
}
void  QDeviceDriverRunDialog::mcomboBox_currentlndexChanged(int index)
{
	CleanTempWidget();
	//qDebug("index %d",index);
	HandleVisaDriverIoctrl(VisaDriverIoctrl(index));
}
void  QDeviceDriverRunDialog::mcomboBox_currentlndexChangedStr(const QString&text)
{

}

void QDeviceDriverRunDialog::HandleVisaDriverIoctrl(VisaDriverIoctrl cmd)
{
	switch (cmd) {
		case VisaDriverIoctrl::IoRead:
		case VisaDriverIoctrl::IoWrite:
			HandleVisaDriverIoctrlIoReadWrite(cmd);
			break;
		case VisaDriverIoctrl::ReadIdentification:
		case VisaDriverIoctrl::ReadSystemError: 
		case VisaDriverIoctrl::ReadSystemErrorCount:
		case VisaDriverIoctrl::ReadQuery:
			HandleVisaDriverIoctrlReadRaw(cmd);
			break;
		case VisaDriverIoctrl::OutputState:
		case VisaDriverIoctrl::SourceCurrentLimit:
		case VisaDriverIoctrl::SourceVoltage:
			HandleVisaDriverIoctrlWithParam(cmd);
			break;
		default:
			break;
	
	}
}

void QDeviceDriverRunDialog::HandleVisaDriverIoctrlIoReadWrite(VisaDriverIoctrl cmd)
{
	auto devlineedit = new QTextEdit;
	ptopVBoxLayout->addWidget(devlineedit);
	tmpwdigetcontain.push_back(devlineedit);
	devlineedit->setText(DEFAULT_DEV);

	auto lineedit = new QTextEdit;
	ptopVBoxLayout->addWidget(lineedit);
	tmpwdigetcontain.push_back(lineedit);
	lineedit->setPlaceholderText("*IDN?");


	auto lineedit_result = new QTextEdit;
	ptopVBoxLayout->addWidget(lineedit_result);
	tmpwdigetcontain.push_back(lineedit_result);
	lineedit_result->setPlaceholderText("result string");

	auto runpb = new QPushButton;
	ptopVBoxLayout->addWidget(runpb);
	tmpwdigetcontain.push_back(runpb);
	runpb->setText(QStringLiteral("运行"));
	connect(runpb, &QPushButton::clicked, this, [this, lineedit, lineedit_result, cmd, devlineedit]() {
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->cmd = cmd;
		if (lineedit->toPlainText().size() > 0) {
			mptr->commond = lineedit->toPlainText().toStdString();
		}
		else {
			mptr->commond = lineedit->placeholderText().toStdString();
		}
		int devid = devlineedit->toPlainText().toInt();
		int ret = TestcaseBase::get_instance()->devcieioctrl(devid, mptr);
		if (ret != 0) {
			lineedit_result->setText(QStringLiteral("运行错误:%1\nDriverResult:[%2]\nMSG:[%3]").arg(ret).arg(mptr->result.c_str()).arg(mptr->to_string().c_str()));
		}
		else {
			lineedit_result->setText(QString("DriverResult:[%1]\nMSG:[%2]").arg(mptr->result.c_str()).arg(mptr->to_string().c_str()));

		}; 
	});
}

void QDeviceDriverRunDialog::HandleVisaDriverIoctrlReadRaw(VisaDriverIoctrl cmd)
{
	auto devlineedit = new QTextEdit;
	ptopVBoxLayout->addWidget(devlineedit);
	tmpwdigetcontain.push_back(devlineedit);
	devlineedit->setText(DEFAULT_DEV);

	auto lineedit_command_echo = new QTextEdit;
	ptopVBoxLayout->addWidget(lineedit_command_echo);
	tmpwdigetcontain.push_back(lineedit_command_echo);
	lineedit_command_echo->setPlaceholderText("null command"); 
	lineedit_command_echo->setReadOnly(true);

	auto lineedit_result = new QTextEdit;
	ptopVBoxLayout->addWidget(lineedit_result);
	tmpwdigetcontain.push_back(lineedit_result);
	lineedit_result->setPlaceholderText("result string");
	lineedit_result->setReadOnly(true);

	auto runpb = new QPushButton;
	ptopVBoxLayout->addWidget(runpb);
	tmpwdigetcontain.push_back(runpb);
	runpb->setText(QStringLiteral("运行"));
	connect(runpb, &QPushButton::clicked, this, [this, lineedit_command_echo, lineedit_result, cmd, devlineedit]() {
		VisaDriverIoctrlBasePtr mptr=nullptr;
		if (cmd == VisaDriverIoctrl::ReadSystemErrorCount) {
			mptr = VisaDriverIoctrlBasePtr(new DeviceDriverReadSystemErrorCount);
		}
		else if (VisaDriverIoctrl::ReadQuery==cmd) {
			mptr = VisaDriverIoctrlBasePtr(new DeviceDriverReadQuery);
		}
		else {
			mptr = VisaDriverIoctrlBasePtr(new VisaDriverIoctrlBase);
			mptr->cmd = cmd;
		}
		//DeviceDriverReadSystemErrorCount* upper_arg = dynamic_cast<DeviceDriverReadSystemErrorCount*>(mptr.get());
		int devid = devlineedit->toPlainText().toInt();
		int ret = TestcaseBase::get_instance()->devcieioctrl(devid,mptr);
		lineedit_command_echo->setText(mptr->commond.c_str());
		if (ret != 0) {
			lineedit_result->setText(QStringLiteral("运行错误:%1\nDriverResult:%2\nMSG:%3").arg(ret).arg(mptr->result.c_str()).arg(mptr->to_string().c_str()));
		}
		else {
			lineedit_result->setText(QString("DriverResult:%1\nMSG:%2").arg(mptr->result.c_str()).arg(mptr->to_string().c_str()));
		}
		});
}

void QDeviceDriverRunDialog::HandleVisaDriverIoctrlWithParam(VisaDriverIoctrl cmd)
{
	auto devlineedit = new QTextEdit;
	ptopVBoxLayout->addWidget(devlineedit);
	tmpwdigetcontain.push_back(devlineedit);
	devlineedit->setText(DEFAULT_DEV);

	auto param_edit = new QLineEdit;
	ptopVBoxLayout->addWidget(param_edit);
	tmpwdigetcontain.push_back(param_edit);
	param_edit->setVisible(false);
	param_edit->setPlaceholderText("parm value");

	auto mcombobox = new QComboBox;
	QStringList mcblist;
	ptopVBoxLayout->addWidget(mcombobox);
	tmpwdigetcontain.push_back(mcombobox);
	mcblist << "Query" << QStringLiteral("设置");
	mcombobox->addItems(mcblist);
	connect(mcombobox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this, param_edit](int index) {
			param_edit->setVisible(index != 0);
		});


	auto lineedit_command_echo = new QTextEdit;
	ptopVBoxLayout->addWidget(lineedit_command_echo);
	tmpwdigetcontain.push_back(lineedit_command_echo);
	lineedit_command_echo->setPlaceholderText("null command");
	lineedit_command_echo->setReadOnly(true);

	auto lineedit_result = new QTextEdit;
	ptopVBoxLayout->addWidget(lineedit_result);
	tmpwdigetcontain.push_back(lineedit_result);
	lineedit_result->setPlaceholderText("result string");
	lineedit_result->setReadOnly(true);

	auto runpb = new QPushButton;
	ptopVBoxLayout->addWidget(runpb);
	tmpwdigetcontain.push_back(runpb);
	runpb->setText(QStringLiteral("运行"));
	connect(runpb, &QPushButton::clicked, this, [mcombobox, param_edit, lineedit_command_echo, lineedit_result, cmd, devlineedit]() {
		VisaDriverIoctrlBasePtr mptr = nullptr;
		if (cmd == VisaDriverIoctrl::OutputState) {
			mptr = VisaDriverIoctrlBasePtr(new DeviceDriverOutputState);
			DeviceDriverOutputState* upper = dynamic_cast<DeviceDriverOutputState*>(mptr.get());
			if (mcombobox->currentIndex() == 0) {
				upper->is_read = true;
			}
			else {
				upper->is_read = false;
				upper->onoff = (param_edit->text().toInt() == 1);
			}
		}
		else if (cmd == VisaDriverIoctrl::SourceCurrentLimit) {
			mptr = VisaDriverIoctrlBasePtr(new DeviceDriverSourceCurrentLimit);
			DeviceDriverSourceCurrentLimit* upper = dynamic_cast<DeviceDriverSourceCurrentLimit*>(mptr.get());
			if (mcombobox->currentIndex() == 0) {
				upper->is_read = true;
			}
			else {
				upper->is_read = false;
				upper->current_ma = param_edit->text().toInt();
			}
		}
		else if (cmd == VisaDriverIoctrl::SourceVoltage) {
			mptr = VisaDriverIoctrlBasePtr(new DeviceDriverSourceVoltage);
			DeviceDriverSourceVoltage* upper = dynamic_cast<DeviceDriverSourceVoltage*>(mptr.get());
			if (mcombobox->currentIndex() == 0) {
				upper->is_read = true;
			}
			else {
				upper->is_read = false;
				upper->voltage_mv = param_edit->text().toInt();
			}
		}
		else {
			mptr = VisaDriverIoctrlBasePtr(new VisaDriverIoctrlBase);
			mptr->cmd = cmd;
		}
		//DeviceDriverReadSystemErrorCount* upper_arg = dynamic_cast<DeviceDriverReadSystemErrorCount*>(mptr.get());
		int devid = devlineedit->toPlainText().toInt();
		int ret = TestcaseBase::get_instance()->devcieioctrl(devid, mptr);
		lineedit_command_echo->setText(mptr->commond.c_str());
		if (ret != 0) {
			lineedit_result->setText(QStringLiteral("运行错误:%1\nDriverResult:%2\nMSG:%3").arg(ret).arg(mptr->result.c_str()).arg(mptr->to_string().c_str()));
		}
		else {
			lineedit_result->setText(QString("DriverResult:%1\nMSG:%2").arg(mptr->result.c_str()).arg(mptr->to_string().c_str()));
		}
		});
}

void QDeviceDriverRunDialog::CleanTempWidget()
{
	for each (auto wdiget in tmpwdigetcontain)
	{
		wdiget->setVisible(false);
		ptopVBoxLayout->removeWidget(wdiget);
	}
	tmpwdigetcontain.clear();
}
