
#include "AutoTestView.h"
#include "ui_GlobalStr.h"
#include "UI_style.h"
#include "Sound.h"
#include "TextEditDelegate.h"
#include <QMessageBox>
#include <QMultInputLineDialog.h>
#include <QComboBox>
#include <QMovie>
#include "TestCaseResultSave.h"
//#include "SliderButton.h"
#define SHOW_LOGWIDGET
AutoTestView::AutoTestView(QWidget* main_widget):mparent_widget(main_widget)
{
	auto parent_layout = new QGridLayout;

	main_widget->setLayout(parent_layout);
	
	//ui.setupUi(this);
	auto title_widget = new QWidget;
	int row_offset = 0;
	int ret = 0;
	//add to parent
	parent_layout->setSpacing(20);
	auto log_widget = new QWidget;
	int total_col = 0;
	{
		parent_layout->addWidget(title_widget, 0, 0, 1, getMainViewGridColum());
		row_offset += 1;
		title_widget->setObjectName("title_widget");
		setuptitleView(title_widget);
		title_widget->setStyleSheet(QString::fromUtf8("#title_widget{border:1px groove gray}"));
	}
	{
		auto device_widget = new QWidget(this);
		int device_view_row = row_offset, device_view_colum = 0;
		int device_view_rowspan = 10;
		int device_view_columspan = 9;
		total_col += device_view_columspan;
		device_widget->setObjectName("device_widget");

		device_widget->setStyleSheet(QString::fromUtf8("#device_widget{border:1px groove gray}"));
		parent_layout->addWidget(device_widget, device_view_row, device_view_colum, device_view_rowspan, device_view_columspan);
		ret=setupdeviceView(device_widget);
		//qDebug("device_widget objectName %s %p/%p ", device_widget->objectName(), device_widget->parent(), mparent_widget);

	}
	{
		auto setting_widget = new QWidget;
		int setting_view_row = row_offset, setting_view_colum = 9;
		int setting_view_rowspan = 10;
		row_offset += setting_view_rowspan;
		int setting_view_columspan = 3;
		total_col += setting_view_columspan;
		setting_widget->setObjectName("setting_widget");
		setting_widget->setStyleSheet(QString::fromUtf8("#setting_widget{border:1px groove gray}"));
		parent_layout->addWidget(setting_widget, setting_view_row, setting_view_colum, setting_view_rowspan, setting_view_columspan);
		setupsettingView(setting_widget);
	}
	{
		auto tctable_widget = new QWidget;
		int tctable_view_row = row_offset, tctable_view_colum = 0;
		int tctable_view_rowspan = 20;
#ifdef SHOW_LOGWIDGET
		int tctable_view_columspan = 9;
#else
		int tctable_view_columspan = total_col;
#endif
		tctable_widget->setObjectName("tctable_widget");
		tctable_widget->setStyleSheet(QString::fromUtf8("#tctable_widget{border:1px groove gray}"));
		parent_layout->addWidget(tctable_widget, tctable_view_row, tctable_view_colum, tctable_view_rowspan, tctable_view_columspan);
		setuptestcaseView(tctable_widget);
	}
#ifdef SHOW_LOGWIDGET	//hide log widget
	{
		auto log_widget = new QWidget;
		int log_view_row = row_offset, log_view_colum = 9;
		int log_view_rowspan = 20;
		int log_view_columspan = 3;
		row_offset += log_view_rowspan;
		log_widget->setObjectName("log_widget");
		log_widget->setStyleSheet(QString::fromUtf8("#log_widget{border:1px groove gray}"));
		parent_layout->addWidget(log_widget, log_view_row, log_view_colum, log_view_rowspan, log_view_columspan);
		setuplogView(log_widget);
	}
#endif
	RegisterSignalTotestcase();
}

AutoTestView::~AutoTestView()
{
	if(GlobalConfig_debugAutoTestView)qInfo(" ");
	OutputDebugStringA("~AutoTestView\r\n");
	//close device device ptr
	//sync result result ptr
	//
}
int32_t AutoTestView::setuplogView(QWidget* parent)
{
	int32_t ret = 0;
	auto log_layout = new QGridLayout;
	parent->setLayout(log_layout);
	auto  log_plaintextedit = new QPlainTextEdit;
	log_layout->addWidget(log_plaintextedit, 0, 0, 1, 1);
	log_plaintextedit->setObjectName("log_plaintextedit");
	log_plaintextedit->setReadOnly(true);
	QPalette p = log_plaintextedit->palette();
	QPalette winp = parent->palette(); //获取parent的palette
	p.setColor(QPalette::Active, QPalette::Base, winp.background().color());
	p.setColor(QPalette::Inactive, QPalette::Base, winp.background().color());
	log_plaintextedit->setPalette(p);
	return ret;
}
int32_t AutoTestView::setuptestcaseView(QWidget* parent)
{
	int32_t ret = 0;
	auto tctable_layout = new QVBoxLayout;//new QGridLayout;
	int total_row = 0;
	int total_colum = 0;
	int colum_item_line = 8;
	int style_colum = colum_item_line + 1 + 1;
	parent->setLayout(tctable_layout);
	QTableWidget* table = new QTableWidget;
	table->setObjectName("testcase_tablewidget");
	//tctable_layout->addWidget(table, 0,0,1,1);//(QWidget *, int row, int column, int rowSpan, int columnSpan)
	tctable_layout->addWidget(table),
	mtcitemcontainer.clear();


#if 1
	//table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    //x先自适应宽度
	//table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//table->resizeRowsToContents();
	//->resizeColumnsToContents();

	//table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif

	table->horizontalHeader()->setVisible(false);
	table->verticalHeader()->setVisible(false);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑

	//Get info for testcase
	int total_step = TestcaseBase::get_instance()->GettestcaseStepcnt();
	int current_row = 0;
	int current_colum = 0;
	//foreach testcase
	for (int stepindex = 0; stepindex < total_step; stepindex++)
	{
		AutoTestCaseitemViewPtr ptr = AutoTestCaseitemView::get_instance(stepindex, current_row);
		if (ptr != nullptr) {
			mtcitemcontainer.push_back(ptr);
			current_row += ptr->GetRowSpan();
			current_colum = ptr->GetColumSpan();
		}		
	}
	if (mtcitemcontainer.size()!= total_step) {
		qCritical("testcaset setup fail");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERR_OUT;
	}
	table->setRowCount(current_row);//nead set first
	table->setColumnCount(current_colum);//index and condition label
	TestCaseTableWidgetFresh();

ERR_OUT:
	return ret;
}

void AutoTestView::RegisterSignalTotestcase()
{

	connect(this, SIGNAL(messagetodevice(int, MessageFVBasePtr)), TestcaseBase::get_instance().get(), SLOT(messagefromview(int, MessageFVBasePtr)));
	connect(TestcaseBase::get_instance().get(), SIGNAL(messagetctoview(int, MessageTVBasePtr)), this, SLOT(messagefromtestcase(int, MessageTVBasePtr)));
}

int32_t AutoTestView::setuptitleView(QWidget* mparent_widget)
{
	int32_t ret = 0;
	auto title_layout = new QGridLayout;
	int columoffset = 0;
	mparent_widget->setLayout(title_layout);
	
	auto label = new QLabel(TITLE_TESTCASE);
	//add to parent
	title_layout->addWidget(label, 0, columoffset++, 1, 1);
	auto label_case = new QLabel;
	label_case->setText(TestcaseBase::get_instance()->get_runcasename());
	//printf("AutoTestView %s", TestcaseBase::get_instance()->get_runcasename().toStdString().c_str());

	auto label_caseversion = new QLabel;
	label_caseversion->setText(TestcaseBase::get_instance()->Get_iniVersion());
	title_layout->addWidget(label_caseversion, 0, columoffset++, 1, 1);

	title_layout->addWidget(label_case, 0, columoffset, 1, 4);
	return ret;
}

int32_t AutoTestView::setupdeviceView(QWidget* parent)
{
	int32_t ret = 0;
	DeviceInfoList deviceinfo;
	TestcaseBase::get_instance()->Get_deviceInfo(deviceinfo);
	int columoffset = 0;
	int cntoffset = 0;
	int totalcolum = 5;
	if(GlobalConfig_debugAutoTestView) qDebug("Get_deviceInfo size %d", deviceinfo.size());
	auto dev_layout = new QGridLayout(this);
	parent->setLayout(dev_layout);
	dev_layout->setHorizontalSpacing(15);
	foreach(auto dev , deviceinfo) {
		auto dev_widget = new QWidget;
		dev_widget->setObjectName(QString("%1%2").arg("dev_widget").arg(cntoffset));
		dev_layout->addWidget(dev_widget, 0, columoffset++, 1, 1);
		QPalette pal(dev_widget->palette());
		//设置背景灰色，未连接
		pal.setColor(QPalette::Background, Qt::lightGray);
		dev_widget->setAutoFillBackground(true);
		dev_widget->setPalette(pal);

		if (stricmp(dev.interfaceid.c_str(),"AUTO")!=0) {
			setdeviceupiteminterface(dev_widget, dev, cntoffset, totalcolum); 
		}
		else {
			setdeviceupitem_auto(dev_widget, dev, cntoffset, totalcolum);
		}
		cntoffset++;
	}

	return ret;
}
int32_t AutoTestView::DeleteView(QWidget* dev_widget)
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
int32_t AutoTestView::setdeviceupitem_auto(QWidget* dev_widget, DeviceInfo_t dev, int32_t cntoffset, int32_t totalcolum)
{
	if (GlobalConfig_debugAutoTestView) qDebug(" ");
	DeleteView(dev_widget);
	auto devitem_layout = new QGridLayout;
	dev_widget->setLayout(devitem_layout);

	auto connect_pb = new QPushButton;
	devitem_layout->addWidget(connect_pb, 2, totalcolum - 2, 1, 2);
	//connect_pb->setObjectName(QString("dev%1_connect_pb").arg(cntoffset));
	connect_pb->setObjectName(QString("devscan%1_connect_pb").arg(cntoffset));
	connect_pb->setVisible(false);
	connect(connect_pb, SIGNAL(clicked()), this, SLOT(on_deviceconectpb_clicked()));
	connect_pb->setText(QStringLiteral("连接"));

	auto networkid_label = new QLabel;
	devitem_layout->addWidget(networkid_label, 2, 0, 1, 2);
	networkid_label->setObjectName(QString("autonewokid_%1_label").arg(cntoffset));
	networkid_label->setText(dev.networklabel.c_str());
	networkid_label->setFont(QFont("Microsoft YaHei UI", 15, QFont::Bold));

	auto deviceres_box = new QComboBox;
	devitem_layout->addWidget(deviceres_box, 1, 0, 1, totalcolum);
	deviceres_box->setObjectName(QString("devscandevcie_%1_combo").arg(cntoffset));
	deviceres_box->setFocusPolicy(Qt::NoFocus);
	connect(deviceres_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this, deviceres_box, connect_pb](int index) {
		QString res = deviceres_box->currentText();
		if (GlobalConfig_debugAutoTestView)qDebug("select index %d ,res %s result %d", index, res.toStdString().c_str(),res.count("Connected")!=0);
		connect_pb->setVisible(res.count("Connected") == 0);
		connect_pb->setStyleSheet("QPushButton{background-color:green;}");
		
		});

	auto scandevice_pb = new QPushButton;
	devitem_layout->addWidget(scandevice_pb, 0, 0, 1, 2);
	scandevice_pb->setObjectName(QString("devscandevcie_%1_pb").arg(cntoffset));
	scandevice_pb->setText(QStringLiteral("扫描设备"));
	scandevice_pb->setStyleSheet("QPushButton{background-color:green;}");

	auto loadinganima_label = new QLabel;
	loadinganima_label->setObjectName(QString("loadinganima_%1_label").arg(cntoffset));
	devitem_layout->addWidget(loadinganima_label, 0, totalcolum-1, 1, 2);
	loadinganima_label->setVisible(false);

	connect(scandevice_pb, &QPushButton::clicked, this, [this, scandevice_pb, deviceres_box, loadinganima_label, cntoffset]() {
		//FV;start gif
		if (GlobalConfig_debugAutoTestView)qDebug(" ");
		deviceres_box->clear();
		scandevice_pb->setEnabled(false);
		scandevice_pb->setStyleSheet("QPushButton{background-color:gray;}");
		auto m_Move = new QMovie(":/res/image/loading.gif");
		m_Move->setObjectName(QString("loadingmovie_%1").arg(cntoffset));
		loadinganima_label->setMovie(m_Move);
		loadinganima_label->setVisible(true);
		loadinganima_label->setScaledContents(false);
		loadinganima_label->setAlignment(Qt::AlignCenter);
		loadinganima_label->setStyleSheet("QLabel {background-color:transparent;}");//设置lable背景透明
		m_Move->start();
		
		auto scandevmsg = new MessageFVFindDeviceRes;
		MessageFVBasePtr ptr(scandevmsg);
		scandevmsg->index = cntoffset;
		emit messagetodevice(int(ptr->GetCmd()), ptr);
		});
	return 0;
}

int32_t AutoTestView::setdeviceupiteminterface(QWidget* dev_widget, DeviceInfo_t dev, int32_t cntoffset, int32_t totalcolum)
{
	if (GlobalConfig_debugAutoTestView)qDebug("cntoffset %d", cntoffset);
	DeleteView(dev_widget);
	//goto ERR_OUT; //dbg view
	auto devitem_layout = new QGridLayout;
	dev_widget->setLayout(devitem_layout);
	int row_offset = 0;
	int network_span = 2;
	int fontnetwork = 12;
	auto label_network = new QLabel();
	label_network->setText(QString(dev.networklabel.c_str()));
	devitem_layout->addWidget(label_network, row_offset++, 0, 1, totalcolum, Qt::AlignCenter);
	label_network->setFont(QFont("Microsoft YaHei UI", fontnetwork, QFont::Bold));
	//label_network->setStyleSheet(QString::fromUtf8("QLabel{border:1px groove gray}"));
	//device class 
	//disable 
	//if (dev.deviceclass == DeviceClass::DeviceClass_Unknow) 
	if(false)
	{
		//V
		int colum_offset = 0;
		int label_span = 1;
		int value_span = totalcolum - label_span;
		auto label_v = new QLabel();
		Qt::Alignment flag = Qt::Alignment();
		label_v->setText("V");
		devitem_layout->addWidget(label_v, row_offset, colum_offset, 1, label_span, flag);
		label_v->setFont(QFont("Microsoft YaHei UI", 20, QFont::Bold));
#if 1
		colum_offset += label_span;
		auto label_vvalue = new QLabel();
		label_vvalue->setText("0.0 V");
		label_vvalue->setObjectName(QString("dev%1_Vvalue").arg(cntoffset));
		label_vvalue->setFont(QFont("Microsoft YaHei UI", 15, QFont::Bold));
		devitem_layout->addWidget(label_vvalue, row_offset++, colum_offset, 1, value_span, Qt::AlignCenter);
#endif
		colum_offset = 0;
		//A
		auto label_A = new QLabel();
		label_A->setText("A");
		devitem_layout->addWidget(label_A, row_offset, colum_offset, 1, label_span, flag);

		label_A->setFont(QFont("Microsoft YaHei UI", 20, QFont::Bold));
#if 1
		colum_offset += label_span;
		auto label_Avalue = new QLabel();
		label_Avalue->setText("0.0 A");
		label_Avalue->setObjectName(QString("dev%1_Avalue").arg(cntoffset));
		label_Avalue->setFont(QFont("Microsoft YaHei UI", 15, QFont::Bold));
		devitem_layout->addWidget(label_Avalue, row_offset++, colum_offset, 1, value_span, Qt::AlignCenter);
#endif
	}

	{
		auto devlast_widget = new QWidget;

		auto devlast_layout = new QGridLayout;
		devlast_widget->setLayout(devlast_layout);
		devlast_widget->setObjectName(QString("dev%1_last_widget").arg(cntoffset));
		devlast_widget->setStyleSheet(QString("#dev%1_last_widget{border:1px groove gray}").arg(cntoffset));
		devitem_layout->addWidget(devlast_widget, row_offset, 0, 1, totalcolum);

		auto devinfo_widget = new QWidget;
		int devinfo_span = 4;
		int devctr_span = totalcolum - devinfo_span;
		devlast_layout->addWidget(devinfo_widget, 0, 0, 1, devinfo_span);
		auto devinfo_layout = new QGridLayout;
		devinfo_widget->setLayout(devinfo_layout);

		auto label_iden = new QLabel();
		label_iden->setText(QString("%1 %2").arg(QStringLiteral("设备名称")).arg(dev.identify.c_str()));
		label_iden->setWordWrap(true);
		label_iden->setAlignment(Qt::AlignTop);
		int last_row_offset = 0;
		devinfo_layout->addWidget(label_iden, last_row_offset++, 0, 1, 1);
		auto label_interface = new QLabel();
		label_interface->setText(QString("%1 %2").arg(QStringLiteral("连接方式")).arg(dev.interfaceid.c_str()));
		devinfo_layout->addWidget(label_interface, last_row_offset, 0, 1, 1);
		label_interface->setWordWrap(true);
		label_interface->setAlignment(Qt::AlignTop);


		auto devctr_widget = new QWidget;
		devlast_layout->addWidget(devctr_widget, 0, devinfo_span, 1, devctr_span);
		auto devctr_layout = new QGridLayout;
		devctr_widget->setLayout(devctr_layout);
#if 1
		auto connect_pb = new QPushButton(this);
		connect_pb->installEventFilter(this);
		connect_pb->setObjectName(QString("dev%1_connect_pb").arg(cntoffset));
		connect_pb->setText(QStringLiteral("连接"));
#else	//太丑了
		auto connect_pb = new SliderButton;
		connect_pb->setObjectName(QString("dev%1_connect_pb").arg(cntoffset));
		//connect_box->setText(QStringLiteral("连接"));

#endif
		last_row_offset = 0;
		devctr_layout->addWidget(connect_pb, last_row_offset++, 0, 1, 1);
		connect(connect_pb, SIGNAL(clicked()), this, SLOT(on_deviceconectpb_clicked()));
		DeviceConnectWdigetFresh(cntoffset, connect_pb);//end view

		auto testactvie_pb = new QPushButton(this);
		testactvie_pb->setObjectName(QString("dev%1_testactive_pb").arg(cntoffset));
		testactvie_pb->setText(QStringLiteral("检测连接"));
		testactvie_pb->installEventFilter(this);
		DeviceActiveWdigetFresh(cntoffset, testactvie_pb);
		devctr_layout->addWidget(testactvie_pb, last_row_offset, 0, 1, 1);
		connect(testactvie_pb, SIGNAL(clicked()), this, SLOT(on_devicetestactivepb_clicked()));
	}
ERR_OUT:
	return 0;
}
int32_t AutoTestView::setupsettingView(QWidget* parent)
{
	int ret = 0;
	auto setting_layout = new QGridLayout;
	int columoffset = 0;
	setting_layout->setVerticalSpacing(25);
	parent->setLayout(setting_layout);
	int row_offset = 0;
	int total_colum = 3;
	auto cycle_option = new QCheckBox(QStringLiteral("循环测试"));
	cycle_option->installEventFilter(this);
	cycle_option->setObjectName("cycle_option");
	connect(cycle_option, &QCheckBox::stateChanged, this, [this](int value) {
		if (Qt::Checked == value) {
			msettings.is_cycle = true;
		}
		else {
			msettings.is_cycle = false;
		}
		AutoTestSettingWidgetFresh();
		//qDebug("cycle_option QCheckBox::stateChanged to %d", msettings.is_cycle);
		});
	setting_layout->addWidget(cycle_option, row_offset, 0,1,1);

	auto test_start_pb = new QPushButton;
	test_start_pb->setObjectName("test_start_pb");
	connect(test_start_pb, SIGNAL(clicked()), this, SLOT(on_test_start_pb_clicked()));
	setting_layout->addWidget(test_start_pb, row_offset, total_colum-1, 1, 1);
	TestCastStartPauseWidgetFresh(test_start_pb,false);
	{
	//boardsn
		auto boardsn_lineedit = new QLineEdit;
		setting_layout->addWidget(boardsn_lineedit, row_offset, 1, 1, 1);
		boardsn_lineedit->setPlaceholderText(QStringLiteral("测试板编号"));
		QRegExp rx("^[a-zA-Z]*\\d+");
		boardsn_lineedit->setValidator(new QRegExpValidator(rx));
		boardsn_lineedit->setObjectName("test_boardsn");

	}
	row_offset++;
//
	{
		auto cycleintervalbox = new QSpinBox(this);
		cycleintervalbox->setObjectName("cycleinterval_box");
		cycleintervalbox->setMinimum(5);
		cycleintervalbox->setSuffix(" S");
		cycleintervalbox->setPrefix(QStringLiteral("循环间隔"));
		cycleintervalbox->setValue(msettings.cycleintervalsecond);
		cycleintervalbox->installEventFilter(this);
		cycleintervalbox->setSingleStep(5);
		cycleintervalbox->setFocusPolicy(Qt::NoFocus);
		connect(cycleintervalbox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
			QSpinBox* box = qobject_cast<QSpinBox*>(sender());
			msettings.cycleintervalsecond = box->value();
			if (GlobalConfig_debugAutoTestView)qDebug("updata cycleintervalsecond %d", msettings.cycleintervalsecond);
			});
		setting_layout->addWidget(cycleintervalbox, row_offset, 0, 1, 1);
	}
	auto test_termin_pb = new QPushButton;
	test_termin_pb->setObjectName("test_termin_pb");
	connect(test_termin_pb, SIGNAL(clicked()), this, SLOT(on_test_termin_pb_clicked()));
	setting_layout->addWidget(test_termin_pb, row_offset++, total_colum - 1, 1, 1);
	TestCastStartTerminWidgetFresh(test_termin_pb,false);

	//settings Status 
	{
		auto label_cyclestatus = new QLabel();
		label_cyclestatus->setObjectName("label_cyclestatus");
		setting_layout->addWidget(label_cyclestatus, row_offset, 0, 1, 1);

		auto test_clean_cyclecnt_pb = new QPushButton;
		test_clean_cyclecnt_pb->setObjectName("test_clean_cyclecnt_pb");
		setting_layout->addWidget(test_clean_cyclecnt_pb, row_offset++, total_colum - 1, 1, 1);
		test_clean_cyclecnt_pb->setText(QStringLiteral("清除历史数据"));
		test_clean_cyclecnt_pb->installEventFilter(this);
		TestCastcycleStatusCleanWidgetFresh(test_clean_cyclecnt_pb);
		connect(test_clean_cyclecnt_pb, &QPushButton::clicked, this, [this]() {
			qInfo("clean msettings S %d UT %d F %d", msettings.success_cnt, msettings.usertermin_cnt, msettings.fail_cnt);
			msettings.fail_cnt = 0;
			msettings.success_cnt = 0;
			msettings.usertermin_cnt = 0;
			TestCaseResultSaveICMP::get_instance()->SyncToDisk();
			TestCastcycleStatusWidgetFresh();
			});
	}
	auto label_status = new QLabel();
	label_status->setObjectName("label_status");
	setting_layout->addWidget(label_status, row_offset++, 0, 1, total_colum);
	//setting_layout-
	label_status->setStyleSheet("#label_status{background-color:green;}");
	AutoTestSettingWidgetFresh();
	return ret;
}
void AutoTestView::AutoTestSettingWidgetFresh()
{
	auto cycle_option = mparent_widget->findChild<QCheckBox*>(QString("cycle_option"));
	if (cycle_option) {
		cycle_option->setChecked(msettings.is_cycle);
	}
	
	auto cycleinterval_box = mparent_widget->findChild<QSpinBox*>(QString("cycleinterval_box"));
	if (cycleinterval_box) {
		cycleinterval_box->setVisible(msettings.is_cycle);
	}
	else {
	
	}
	TestCastcycleStatusCleanWidgetFresh();
	TestCastcycleStatusWidgetFresh();
}
bool AutoTestView::TestCastcycleStatusCleanWidgetFresh(QPushButton* pb)
{
	QPushButton* mpb = pb;
	if (pb == nullptr) {
		mpb = mparent_widget->findChild<QPushButton*>(QString("test_clean_cyclecnt_pb"));
	}
	if (mpb == nullptr)goto ERR_OUT;
	mpb->setVisible(msettings.is_cycle);
ERR_OUT:
	return true;
}
bool AutoTestView::TestCaseTableWidgetFresh(QTableWidget* pb)
{
	bool ret = true;
	QTableWidget* table = pb;
	if (pb ==nullptr) {
		table = mparent_widget->findChild<QTableWidget*>(QString("testcase_tablewidget"));
	}
	if (table == nullptr) {
		ret = false; goto ERR_OUT;
	}
	{
		if (GlobalConfig_debugAutoTestView)qDebug(" ");
#if 1
		QPalette p = table->palette();
		QPalette winp = mparent_widget->palette(); //获取parent的palette
		p.setColor(QPalette::Active, QPalette::Base, winp.background().color());
		p.setColor(QPalette::Inactive, QPalette::Base, winp.background().color());
		table->setPalette(p);
#endif
	}

	int total_step = TestcaseBase::get_instance()->GettestcaseStepcnt();
	for (int stepindex = 0; stepindex < total_step; stepindex++) {
		{
			//index
			int32_t row, colum, rowspan, columspan = 0;
			auto item = mtcitemcontainer[stepindex];
			item->ResetCursor();
			item->GetindexRect(row, colum, rowspan, columspan);
			QTableWidgetItem* index = table->item(row, colum);
			if(index==nullptr) {
				if (rowspan > 1 || columspan > 1)table->setSpan(row, colum, rowspan, columspan);
				index = new QTableWidgetItem;
				if (GLOBALSETTINGSINSTANCE->isUserRoot()) {
					index->setCheckState(Qt::Checked);
				}
				table->setItem(row, colum, index);
				index->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
				index->setText(QString::asprintf("%03d", stepindex));
			}
		}
		{
			//uiteleg
			int32_t row, colum, rowspan, columspan = 0;
			auto item = mtcitemcontainer[stepindex];
			{
				item->GetuitlegcontionRect(row, colum, rowspan, columspan);
				QTableWidgetItem* witem = table->item(row, colum);
				if (witem == nullptr) {
					witem = new QTableWidgetItem;
					if (rowspan > 1 || columspan > 1)table->setSpan(row, colum, rowspan, columspan);
					table->setItem(row, colum, witem);
					witem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					witem->setText(QStringLiteral("操作步骤"));
				}
			}
			{
				item->GetuitlegresultRect(row, colum, rowspan, columspan);

				QTableWidgetItem* witem = table->item(row, colum);
				if (witem == nullptr) {
					witem = new QTableWidgetItem;
					if (rowspan > 1 || columspan > 1)table->setSpan(row, colum, rowspan, columspan);
					table->setItem(row, colum, witem);
					witem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					witem->setText(QStringLiteral("测试数据"));
				}
			}
		}
		{
			//comment
			int32_t row, colum, rowspan, columspan = 0;
			auto item = mtcitemcontainer[stepindex];
			item->GetcommentRect(row, colum, rowspan, columspan);
			QTableWidgetItem* witem = table->item(row, colum);
			if (witem == nullptr) {
				witem = new QTableWidgetItem;
				if(rowspan>1|| columspan>1)table->setSpan(row, colum, rowspan, columspan);
				table->setItem(row, colum, witem);
				witem->setTextAlignment(Qt::AlignLeft);
			}
			ShowString comment_str;
			TestcaseBase::get_instance()->Gettestcasetypeshow(stepindex, caseitem_class::Comment, comment_str);
			witem->setText(comment_str);
#if 1
			{
				//chec span
				int checkspan_row = row; int checkspan_colum = colum - 1;
				QTableWidgetItem* witem = table->item(checkspan_row, checkspan_colum);
				if (witem == nullptr) {
					witem = new QTableWidgetItem;
					table->setItem(checkspan_row, checkspan_colum, witem);
				}
			}
#endif
		}
		{
			//flow
			//他遍历caseitem。获取位置靠AutoTestCaseitemView
			auto item = mtcitemcontainer[stepindex];
			int total = TestcaseBase::get_instance()->Gettestcaseitemstotal(stepindex);
			int max_colum = 0;
			int current_row=0;
			for (int i = 0; i < total; i++) {
				int32_t row, colum, rowspan, columspan = 0;
				ret = item->GetcurrentCursorRect(row, colum, rowspan, columspan);
				//合法数据
				if (ret == 0) {
					if (max_colum < colum) { 
						if (stepindex==0) {
							current_row = row;
						}
						max_colum = colum; 
					}
					QTableWidgetItem* witem = table->item(row, colum);
					if (witem == nullptr) {
						witem = new QTableWidgetItem;
						table->setItem(row, colum, witem);
						witem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					}
					else {
						auto color = witem->backgroundColor();
						color.setAlpha(0);
						witem->setBackgroundColor(color);
					}
					ShowString comment_str;
					TestcaseBase::get_instance()->Gettestcaseitemshow(stepindex, i, comment_str);
					witem->setText(comment_str);
					auto textEditer = table->itemDelegateForColumn(colum); 
					if (textEditer==nullptr) {
						textEditer = new TextEditDelegate;
						//给表格设置代理
						table->setItemDelegateForColumn(colum, textEditer);
					}

					{
						table->resizeRowToContents(row);//行
					}
				}
				item->CursorNext();
			}
			if (max_colum<(table->columnCount()-1) && stepindex==0) {
				//push empty item

				while (++max_colum< table->columnCount()) {
					QTableWidgetItem* witem = table->item(current_row, max_colum);
					if (GlobalConfig_debugAutoTestView)qDebug("virtual item on %d ~ %d", current_row, max_colum);
					if (witem == nullptr) {
						witem = new QTableWidgetItem;
						table->setItem(current_row, max_colum, witem);
					}
				}
			}
		}
	}
ERR_OUT:
	return ret;
}
bool AutoTestView::TestCastcycleStatusWidgetFresh(QLabel* label)
{
	QLabel* mlabel = label;
	if (label == nullptr) {
		mlabel = mparent_widget->findChild<QLabel*>(QString("label_cyclestatus"));
	}
	if (mlabel == nullptr)goto ERR_OUT;
	mlabel->setText(QStringLiteral("已成功:%1 \n失败: %2 \n终止 %3").arg(msettings.success_cnt).arg(msettings.fail_cnt) \
		.arg(msettings.usertermin_cnt));
	mlabel->setVisible(msettings.is_cycle);
ERR_OUT:
	return true;
}
bool AutoTestView::TestCastStartTerminWidgetFresh(QPushButton* pb,bool isruncase)
{
	bool ret = true;
	if (pb == nullptr)return false;
	pb->setText(QStringLiteral("终止测试"));
	if (!isruncase) {
		pb->setStyleSheet("QPushButton{background-color:gray;}");
	}
	else {
		pb->setStyleSheet("QPushButton{background-color:green;}");
	}
	pb->setEnabled(true);
	return ret;
}
bool AutoTestView::TestCastStartPauseWidgetFresh(QPushButton* pb, bool isruncase)
{
	bool ret = true;
	if (pb == nullptr)return false;
	if (!isruncase) {
		pb->setText(QStringLiteral("开始测试"));
		pb->setStyleSheet("QPushButton{background-color:gray;}");
	}
	else {
		pb->setText(QStringLiteral("暂停测试"));
		pb->setStyleSheet("QPushButton{background-color:green;}");
	}
	pb->setEnabled(true);
	return ret;
}
bool AutoTestView::eventFilter(QObject* watched, QEvent* event)
{
	bool stopevnt = false;
	//qDebug("type %d", event->type());
	if (event->type() == QEvent::MouseButtonPress ||
		event->type() == QEvent::MouseButtonDblClick ) {
		if (GlobalConfig_debugAutoTestView)qDebug("type %d", event->type());
		if (TestcaseBase::get_instance()->isRuncase()) {
			stopevnt = true;
		}
	}
	if (stopevnt) {
		QMessageBox::warning(this, WARN_STR, QStringLiteral("正在测试中，请先停止测试"), QMessageBox::Yes);
	}
	return !stopevnt?QWidget::eventFilter(watched, event): stopevnt;
}
void AutoTestView::DeviceConnectWdigetFresh(int32_t index, QPushButton* pb)
{
	DeviceStatus_t status;
	TestcaseBase::get_instance()->Getdevicestatus(index, status);
	auto connect_pb = mparent_widget->findChild<QPushButton*>(QString("dev%1_connect_pb").arg(index));

	if (connect_pb == nullptr) {
		if (!status.connected) { 
			auto scanconnect_pb=mparent_widget->findChild<QPushButton*>(QString("devscan%1_connect_pb").arg(index));
			if (scanconnect_pb) {
				scanconnect_pb->setEnabled(true);
			}
			return; 
		};
		auto dev_widget = mparent_widget->findChild<QWidget*>(QString("%1%2").arg("dev_widget").arg(index));
		DeviceInfoList deviceinfo;
		TestcaseBase::get_instance()->Get_deviceInfo(deviceinfo,index);
		DeviceInfo_t dev = deviceinfo.front();
		setdeviceupiteminterface(dev_widget, dev, index, 5);
		connect_pb = mparent_widget->findChild<QPushButton*>(QString("dev%1_connecdt_pb").arg(index));
	}

	if (pb)connect_pb = pb;
	if (!connect_pb) {
		qCritical("null pb index %d",index);
		return;
	}
	if (GlobalConfig_debugdevciewidget)qDebug("pb index %d status %d", index, status.connected);
	//connect_pb->setStyleSheet(DEVICE_BUTTON_OFF_STYLE);
	if (status.connected) {
		connect_pb->setStyleSheet(DEVICE_BUTTON_ON_STYLE);
	}
	else {
		connect_pb->setStyleSheet(DEVICE_BUTTON_OFF_STYLE);
	}
	connect_pb->setEnabled(true);
}
void AutoTestView::DeviceActiveWdigetFresh(int32_t index, QPushButton*pb)
{
	DeviceStatus_t status;
	TestcaseBase::get_instance()->Getdevicestatus(index, status);
	auto mpb = mparent_widget->findChild<QPushButton*>(QString("dev%1_testactive_pb").arg(index));
	if (pb)mpb = pb;
	if (!mpb) {
		qCritical("null mpb index %d", index);
		return;
	}
	if (GlobalConfig_debugdevciewidget)qDebug("mpb index %d status %d", index, status.connected);
	if (status.connected) {
		
		mpb->setStyleSheet(DEVICE_BUTTON_ON_STYLE);
		mpb->setEnabled(true);
	}
	else {
		
		mpb->setStyleSheet(DEVICE_BUTTON_OFF_STYLE);
		mpb->setEnabled(false);
	}
}
void AutoTestView::DeviceWidgetFresh(int32_t index, MessageTVDeviceUpdate* msg)
{
	DeviceStatusIcon icon = msg->icon;
	if(GlobalConfig_debugdevciewidget)qDebug("index %d icon %d",index, icon);
		switch (icon) {
		case DeviceStatusIcon::Connect:
			DeviceConnectWdigetFresh(index);
			DeviceActiveWdigetFresh(index);
			break;
		case DeviceStatusIcon::TestActive:
			Beep(do, 400);
			break;
		default:
			qCritical("icon %d unknown", icon);
			break;
		}

}
void AutoTestView::TestCaseBGWidgetFresh(MessageTVBGUpdate* msg)
{
	bool  isrun = false;

	//auto get
	isrun = TestcaseBase::get_instance()->isRuncase();
	bool ispause = TestcaseBase::get_instance()->isPausecase();
	auto test_start_pb = mparent_widget->findChild<QPushButton*>(QString("test_start_pb"));
	if (test_start_pb)TestCastStartPauseWidgetFresh(test_start_pb, ispause?!ispause:isrun);
	auto test_termin_pb = mparent_widget->findChild<QPushButton*>(QString("test_termin_pb"));
	if (test_termin_pb)TestCastStartTerminWidgetFresh(test_termin_pb, isrun);
	TestingBoardSNWidgetFresh();
}
void AutoTestView::HandleTestCaseOneShot(MessageTVBGStatus* msg)
{
	bool issuccess = false;
	bool isusertermin = false;
	if (GlobalConfig_debugAutoTestView)qDebug(" ");
	if (msg == nullptr) {
		issuccess = TestcaseBase::get_instance()->GetTestcaseBGStep() == TestStep::Complete;
		isusertermin = TestcaseBase::get_instance()->GetTestcaseBGStep() == TestStep::UserTermin;
	}
	else {
		issuccess = msg->issuccess;
		isusertermin = msg->isusertermin;
	}
	QString msg_str=QStringLiteral("本轮测试失败");
	if (issuccess ) {
		msg_str = QStringLiteral("本轮测试正常");
		msettings.success_cnt++;
	}
	else if (isusertermin){
		msg_str = QStringLiteral("本轮测试用户终止");
		msettings.usertermin_cnt++;
	}
	else {
		msettings.fail_cnt++;
	}
	auto status_label = mparent_widget->findChild<QPushButton*>(QString("label_status"));
	if (status_label) {
		if (issuccess || isusertermin) {
			status_label->setStyleSheet("#label_status{background-color:green;}");
		}
		else {
			status_label->setStyleSheet("#label_status{background-color:red;}");
		}
	}
	TestCaseBGWidgetFresh();
	AutoTestSettingWidgetFresh();
	if (TestCaseResultSaveICMP::get_instance()->isSaveAsTable()) {
		HandleTestCaseResultSave();
	}
	QMessageBox::warning(this, INFO_STR, msg_str, QMessageBox::Yes);
	
	if(issuccess)CycleTestHandle();
}
void AutoTestView::HandleTestCaseResultSave(int sector,int seek,QString value,QColor c)
{
	auto table = mparent_widget->findChild<QTableWidget*>(QString("testcase_tablewidget"));
	QString prex = GetTestingBoardSN();
	TestCaseResultSaveICMP::get_instance()->Save(sector, seek, prex, value,c);

}
void AutoTestView::HandleTestCaseResultSave()
{
	auto table = mparent_widget->findChild<QTableWidget*>(QString("testcase_tablewidget"));
	QString prex = GetTestingBoardSN();
	TestCaseResultSaveICMP::get_instance()->Save(table, prex);
}
void AutoTestView::CycleTestHandle()
{
	if (msettings.is_cycle) {
		QCountDownDialog* dlg = new QCountDownDialog(this,2);
		QString msg = QStringLiteral("请在倒计时完成前,更换被测试主板 \n并按 确认 按钮");
		int res = dlg->Run(msettings.cycleintervalsecond*1000, msg);
		if (GlobalConfig_debugAutoTestView)qDebug("dlg res %d", res);
		if (res == QDialog::Accepted) {
			TestingBoardSNWidgetIncrease();
			on_test_start_pb_clicked();
		}
		delete dlg;
	}
}
void AutoTestView::HandleCountDonwDialog(MessageTVCaseCountDownDialog* msg)
{
	if (msg == nullptr)return;
	QCountDownDialog* dlg = new QCountDownDialog(NULL,1,msg->resource);
	int res = dlg->Run(msg->durationms, msg->msg);
	if (GlobalConfig_debugAutoTestView)qDebug("dlg res %d", res);
	msg->is_success = (res != QDialog::Rejected);
	msg->mwait.notify_all();
	delete dlg;
}
void AutoTestView::HandleCaseNoticeDialog(MessageTVCaseNoticeDialog* msg)
{

	if (msg == nullptr)return;
	QMessageBox::StandardButton result = QMessageBox::information(this, INFO_STR, msg->msg, QMessageBox::Yes);
	msg->mwait.notify_all();
	if (GlobalConfig_debugAutoTestView)qDebug("result 0x%x", result);
}
void AutoTestView::HandleCaseConfirmDialog(MessageTVCaseConfirmDialog* msg)
{
	if (msg == nullptr)return;
	QMessageBox::StandardButton result;
	if(msg->resource.size()==0){
		result = QMessageBox::question(this, INFO_STR, msg->msg);
	}
	else {
		QCountDownDialog * dlg = new QCountDownDialog(NULL, 2, msg->resource);
		result = QMessageBox::StandardButton(dlg->Run(msg->durationms, msg->msg));
		if (GlobalConfig_debugAutoTestView)qDebug("dlg res %d", result);
		delete dlg;
	}
	msg->buttonclicked = result;
	msg->mwait.notify_all();

	if (GlobalConfig_debugAutoTestView)qDebug("result 0x%x", result);
}

void AutoTestView::HandleCaseConfirmWithinputDialog(MessageTVCaseConfirmWithInputsDialog* msg)
{
	if (msg == nullptr)return;
	msg->input_value.clear();
	QMultInputLineDialog* dlg = new QMultInputLineDialog(msg->prex, "", msg->msg);
	int res = dlg->Run();
	if(res!=0){
		msg->input_value = dlg->GetResult();
	}
	msg->mwait.notify_all();
}

void AutoTestView::HandleCaseItemWidgetStatus(MessageTVCaseItemWidgetStatus* msg)
{
	if (msg == nullptr)return;
	int ret = 0;
	int32_t row, colum;
	//QBrush color = QBrush(QColor(180, 0, 0));
	QColor color = Qt::red;
	if(msg->is_success)color = Qt::green;
	if (msg->sector < mtcitemcontainer.size()) {
		ret = mtcitemcontainer[msg->sector]->GetSeekRec(msg->seek, row, colum);
		if (ret != 0) {
			goto ERR_OUT;//maybe comment
		}
		if(GlobalConfig_debugtestcasetablewidget)qDebug("sector %d/%d rec [%d-%d] ", msg->sector, msg->seek, row,colum);
		auto tctablewidget = mparent_widget->findChild<QTableWidget*>(QString("testcase_tablewidget"));
		if(tctablewidget) {
			auto item = tctablewidget->item(row, colum);
			if (item) {
				item->setBackgroundColor(color);
				if(!TestCaseResultSaveICMP::get_instance()->isSaveAsTable())
				{
					HandleTestCaseResultSave(msg->sector, msg->seek, item->text(), color);
				}
			}
		}
	}
	else {
		qCritical("sector %d outof %d", msg->sector, mtcitemcontainer.size());
		goto ERR_OUT;
	}
ERR_OUT:
	return;
}
void AutoTestView::HandleCaseItemWidgetStringUpdate(MessageTVCaseItemWidgetStringUpdate* msg)
{
	if (msg == nullptr)return;
	int ret = 0;
	int32_t row, colum;
	if (msg->sector < mtcitemcontainer.size()) {
		ret = mtcitemcontainer[msg->sector]->GetSeekRec(msg->seek, row, colum);
		if (ret != 0) {
			goto ERR_OUT;//maybe comment
		}
		//qDebug("sector %d/%d rec [%d-%d] ", msg->sector, msg->seek, row, colum);
		auto tctablewidget = mparent_widget->findChild<QTableWidget*>(QString("testcase_tablewidget"));
		if (tctablewidget) {
			auto item = tctablewidget->item(row, colum);
			//auto item = tctablewidget->itemDelegateForColumn(colum);
			if (item) {
				//qDebug("msg [%s] ", msg->msg.toStdString().c_str());
				if (msg->overwriteMode) {
					item->setText(msg->msg);
				}
				else {
					QString raw = QString("%1\n%2").arg(item->text()).arg(msg->msg);
					item->setText(raw);
				}
				tctablewidget->resizeRowToContents(row);//行
			}
			else {
				qCritical("null item");
			}
		}
	}
	else {
		qCritical("sector %d outof %d", msg->sector, mtcitemcontainer.size());
		goto ERR_OUT;
	}
ERR_OUT:
	return;
}
void AutoTestView::HandleLogWidgetUpdate(MesageTVLogWidgetUpdate* msg)
{
	auto log_plaintextedit = mparent_widget->findChild<QPlainTextEdit*>(QString("log_plaintextedit"));
	if (log_plaintextedit == nullptr)return;
	if (msg == nullptr) {
		if (log_plaintextedit) {
			log_plaintextedit->clear();
		}
		return;
	}
	if (msg->overwriteMode) {
		log_plaintextedit->setPlainText(msg->msg);
	}
	else {
		log_plaintextedit->appendPlainText(msg->msg);
	}

}
void AutoTestView::HandleDeviceScanWidgetUpdate(MessageTVHardWareDeviceRes* msg)
{
if (msg == nullptr)return;
	//stop loading label
	int index = msg->index;
	QStringList mres;
	//return ;//debug anima 
	if (GlobalConfig_debugAutoTestView)qDebug("index %d",index);
	if (GlobalConfig_debugAutoTestView)qDebug("Msg %s", msg->to_string().c_str());
	auto loadinganima = mparent_widget->findChild<QLabel*>(QString("loadinganima_%1_label").arg(index));
	if (loadinganima) {
		if(loadinganima->movie())loadinganima->movie()->stop();
		loadinganima->setVisible(false);
	}
	else {
		qCritical("null loadinganima");
	}
	//combox init

	foreach(auto res,msg->res) {
		mres.push_back(res.c_str());
	}

	auto mcombox = mparent_widget->findChild<QComboBox*>(QString("devscandevcie_%1_combo").arg(index));
	if (mcombox) {
		mcombox->clear();
		mcombox->addItems(mres);
	}
	else {
		qCritical("null devscandevcie combox");
	}
	auto scandevice_pb = mparent_widget->findChild<QPushButton*>(QString("devscandevcie_%1_pb").arg(index));
	if (scandevice_pb != nullptr) {
		scandevice_pb->setEnabled(true);
		scandevice_pb->setStyleSheet("QPushButton{background-color:green;}");
	}
	else {
		qCritical("null devscandevcie_pb");
	}
#if 0
	auto connectcombox_pb = mparent_widget->findChild<QPushButton*>(QString("connectcombox_%1_pb").arg(index));
	if (connectcombox_pb) {
		connectcombox_pb->setVisible(true);
	}
	else {
		qCritical("null connectcombox_pb");
	}
#endif
ERR_OUT:
	return;
}
QString AutoTestView::GetTestingBoardSN()
{
	auto boadsn_lineedit = mparent_widget->findChild<QLineEdit*>(QString("test_boardsn"));
	if (boadsn_lineedit) {
		return boadsn_lineedit->text();
	}
	return "";
}
int32_t AutoTestView::TestingBoardSNWidgetIncrease(int increase)
{
	int ret = 0;
	auto boadsn_lineedit = mparent_widget->findChild<QLineEdit*>(QString("test_boardsn"));
	if (boadsn_lineedit) {
		QString value = boadsn_lineedit->text();
		QRegularExpression rexpre("(?<snprex>[a-zA-Z]*)(?<index>\\d*)");
		QRegularExpressionMatch match = rexpre.match(value);
		if (match.hasMatch()) {
			int index = match.captured("index").toInt();
			index += increase;
			int align = match.captured("index").size() > 0 ? match.captured("index").size() : 3;
			QString raw = QString("%1%2").arg(match.captured("snprex")).arg(index, align, 10, QChar('0'));
			boadsn_lineedit->setText(raw);
		}
		else {
			qCritical("value %s match fail", value.toLocal8Bit().constData());
		}
	}
	return ret;
}
void AutoTestView::TestingBoardSNWidgetFresh()
{
	TestingBoardSNWidgetIncrease(0);
}
void AutoTestView::on_deviceconectpb_clicked()
{
	QPushButton* rb = qobject_cast<QPushButton*>(sender());
	bool isauto = false;
	//dev%1_connect_pb
	rb->palette().color(QPalette::Background);
	std::string name = rb->objectName().toStdString();
	int index = -1,ret=0;
	DeviceStatus_t st;
	ret = sscanf(name.c_str(), "dev%d_connect_pb", &index);
	if (ret != 1) {
		ret = sscanf(name.c_str(), "devscan%d_connect_pb", &index);
		isauto = true;
	}
	if(GlobalConfig_debugpbclickevent)qDebug("name %s ret %d index %d isauto %d", name.c_str(),ret,index, isauto);
	if (ret == 1) {
		TestcaseBase::get_instance()->Getdevicestatus(index, st);
		auto packet = new MessageFVDeviceConnectDis;
		packet->index = index;

		if (isauto) {
			auto mcombox = mparent_widget->findChild<QComboBox*>(QString("devscandevcie_%1_combo").arg(index));
			if (mcombox) {
				packet->customerinterfaceid = mcombox->currentText().toStdString();
			}
		}
		if (GlobalConfig_debugAutoTestView)qDebug("customerinterfaceid %s", packet->customerinterfaceid.c_str());
		MessageFVBasePtr packetptr(packet);
		if (st.connected) {
			packet->isconnect = false;
		}
		else {
			packet->isconnect = true;
		}
		rb->setEnabled(false);
		emit messagetodevice(int(MessageFromView::ConnectDisDevice), packetptr);
	}
}
void AutoTestView::on_devicetestactivepb_clicked()
{
	QPushButton* rb = qobject_cast<QPushButton*>(sender());
	//dev%1_testactive_pb
	rb->palette().color(QPalette::Background);
	std::string name = rb->objectName().toStdString();
	int index = 0, ret = 0;
	ret = sscanf(name.c_str(), "dev%d_testactive_pb", &index);
	if (GlobalConfig_debugpbclickevent)qDebug("name %s ret %d index %d", name.c_str(), ret, index);
	if (ret == 1) {
		auto packet = new MessageFVBase;
		packet->index = index;
		packet->cmd = MessageFromView::TestactiveDevice;
		MessageFVBasePtr packetptr(packet);
		
		emit messagetodevice(int(MessageFromView::TestactiveDevice), packetptr);
	}
}
void AutoTestView::on_test_start_pb_clicked()
{
	QPushButton* rb = qobject_cast<QPushButton*>(sender());
	if (!rb) {
		rb = mparent_widget->findChild<QPushButton*>(QString("test_start_pb"));
	}
	if (TestcaseBase::get_instance()->InterfaceidSetable()) {
		QMessageBox::warning(NULL,WARN_STR,QStringLiteral("请先连接设备"));
		return;
	}
	auto packet = new MessageFVBase;
	packet->cmd = MessageFromView::TestCasePauseRun;
	MessageFVBasePtr packetptr(packet);
	if (!TestcaseBase::get_instance()->isRuncase()) {
		HandleLogWidgetUpdate();
		TestCaseTableWidgetFresh();
	}
	if(GLOBALSETTINGSINSTANCE->isUserRoot()) {
		QList<int> active = {};
		auto table = mparent_widget->findChild<QTableWidget*>(QString("testcase_tablewidget"));
		if(table){
			foreach(auto caseitem, mtcitemcontainer) {
				int32_t row, colum, rowspan, columspan = 0;
				caseitem->GetindexRect(row, colum, rowspan, columspan);
				auto tcitem = table->item(row,colum);
				if (tcitem && tcitem->checkState() == Qt::Checked) {
					active.push_back(caseitem->GetOffset());
				}
			}
			TestcaseBase::get_instance()->SetActiveCaseSector(active);
		}
		else {
			qCritical("null testcase_tablewidget");
		}
	}
	emit messagetodevice(int(packetptr->GetCmd()), packetptr);
	//Lock clicked test_start_pb
	if(rb)rb->setEnabled(false);
}
void AutoTestView::on_test_termin_pb_clicked()
{
	QPushButton* rb = qobject_cast<QPushButton*>(sender());
	//先判断下状态
	bool isrun = TestcaseBase::get_instance()->isRuncase();
	if (GlobalConfig_debugAutoTestView)qDebug("is_run %d",isrun);
	if(isrun){
		auto packet = new MessageFVBase;
		packet->cmd = MessageFromView::TestCaseTermin;
		MessageFVBasePtr packetptr(packet);
		emit messagetodevice(int(packetptr->GetCmd()), packetptr);
		rb->setEnabled(false);
	}
	else {
		//log 
		TestCaseResultSaveICMP::get_instance()->SyncToDisk();
		//qDebug("close %d",close());
		QApplication::exit();
	}
}
void AutoTestView::messagefromtestcase(int cmd, MessageTVBasePtr ptr)
{
	MessageToView vcmd = MessageToView(cmd);
	auto devp = ptr.get();

	switch (vcmd) {
	case MessageToView::DeviceStatusUpdate:
		{
			MessageTVDeviceUpdate* msg = dynamic_cast<MessageTVDeviceUpdate*>(devp);
			DeviceWidgetFresh(msg->index, msg);		
		}
			break;
	case MessageToView::TestCaseBGStatusUpdate:
		{
			MessageTVBGUpdate* msg = dynamic_cast<MessageTVBGUpdate*>(devp);
			TestCaseBGWidgetFresh(msg);
		}
		break;
	case MessageToView::TestCaseBGOneShot:
		{
			MessageTVBGStatus* msg = dynamic_cast<MessageTVBGStatus*>(devp);
			HandleTestCaseOneShot(msg);
		}
		break;
	case MessageToView::TestCaseHandleCountdownDialog:
	{
		MessageTVCaseCountDownDialog *msg = dynamic_cast<MessageTVCaseCountDownDialog*>(devp);
		HandleCountDonwDialog(msg);
	}
	break;
	case MessageToView::TestCaseHandleManualNoticeDialog:
	{
		MessageTVCaseNoticeDialog* msg = dynamic_cast<MessageTVCaseNoticeDialog*>(devp);
		HandleCaseNoticeDialog(msg);
	}
	break;
	case MessageToView::TestCaseHandleManualConfirmDialog:
	{
		MessageTVCaseConfirmDialog*msg = dynamic_cast<MessageTVCaseConfirmDialog*>(devp);
		HandleCaseConfirmDialog(msg);
	}
	break;
	case MessageToView::TestCaseItemStatus:
	{
		MessageTVCaseItemWidgetStatus* msg = dynamic_cast<MessageTVCaseItemWidgetStatus*>(devp);
		HandleCaseItemWidgetStatus(msg);
	}
	break;
	case MessageToView::TestCaseHandleManualConfirmWithInputsDialog:
	{
		MessageTVCaseConfirmWithInputsDialog* msg = dynamic_cast<MessageTVCaseConfirmWithInputsDialog*>(devp);
		HandleCaseConfirmWithinputDialog(msg);
	}
	break;
	case MessageToView::TestCaseItemStringMsgUpdate:
	{
		MessageTVCaseItemWidgetStringUpdate* msg = dynamic_cast<MessageTVCaseItemWidgetStringUpdate*>(devp);
		HandleCaseItemWidgetStringUpdate(msg);
	}
	break;
	case  MessageToView::HardWareDeviceRes:
	{
		MessageTVHardWareDeviceRes* msg = dynamic_cast<MessageTVHardWareDeviceRes*>(devp);
		HandleDeviceScanWidgetUpdate(msg);
	}
	break;
	case MessageToView::BackGroundServiceMsgToLogWidget:
	{
		MesageTVLogWidgetUpdate* msg = dynamic_cast<MesageTVLogWidgetUpdate*>(devp);
		HandleLogWidgetUpdate(msg);
	}
		break;
	default:
		qCritical("Unkown vcmd %d", vcmd);
		break;
	}
}
