#pragma once
#include <memory>
#include <QString>
#include <QSettings>
#include "CWinLock.h"
#include "DeviceBase.h"
#include "CaseItemBase.h"
#include "workController.h"
#include "MessagePacket.h"
#include "TestCaseBGServer.h"
#include "GlobalConfig.h"
class TestcaseBase;
typedef std::shared_ptr<TestcaseBase> TestcaseBasePtr;
using DeviceInfoList = std::list<DeviceInfo_t>;
typedef std::list<MessageFVBasePtr> MessageFromViewContainer;
typedef std::list<MessageToView> MessageToViewContainer;
typedef QMap<QString, QString> INICommontagContainer;
class TestcaseBase:public QObject	//,public TestCaseBGServer,
{
	Q_OBJECT
	friend class TestCaseBGServer;
	enum class settingsgroup_class
	{
		group_class_unknow =0,
		group_class_iniversion,
		group_class_commontag,
		group_class_device,
		group_class_testcaseitem,
	};

signals:
	void messagetctoview(int, MessageTVBasePtr);
public slots:
	void messagefromview(int, MessageFVBasePtr);
	void messagefromdevice(int, MessageTVBasePtr);
	void messagefromBG(int, MessageTVBasePtr);
	void messagefromcaseitem(int, MessageTVBasePtr);
public:
	static TestcaseBasePtr get_instance();
	bool LoadFile(QString filename);

	QString get_runcasename();
	//virtual int32_t transSettings(QSettings* settings)=0;

public:
	//view 
	QString Get_iniVersion() {
		return QString(ini_version.c_str());
	};

	int32_t Get_deviceInfo(DeviceInfoList & info, int32_t offset=-1);
	int32_t GetDeviceId(std::string networkid, int32_t& id);
	bool InterfaceidSetable(int32_t offset = -1);
	//Resourcecontainer FindDeviceRes();
	int32_t devcieioctrl(int32_t index, VisaDriverIoctrlBasePtr ptr = nullptr);

	int32_t GettestcaseStepcnt();

	int32_t Gettestcasetconditionitemcnt(int32_t offset_testcase);
	int32_t Gettestcasetresultitemcnt(int32_t offset_testcase);
	int32_t Gettestcaseitemstotal(int32_t offset_testcase);
	int32_t Gettestcaseitemshow(int offset_testcase, int offset_item, ShowString & output);
	int32_t Gettestcasetypeshow(int offset_testcase, caseitem_class type, ShowString & output);
	QString GettestcasetConditionShowTotal(int32_t offset_testcase);
	caseitem_type Gettestcaseitemtype(int offset_testcase, int offset_item);

public:

	int32_t Getdevicestatus(int32_t index, DeviceStatus_t &st);

	TestStep GetTestcaseBGStep();
	bool isRuncase();
	bool isPausecase();

	QString GetCommontag(QString key);
private:
	int32_t deviceconnectdis(int32_t index,bool isconnect,std::string customerinterfaceid);
	int32_t devicetestactive(int32_t index);


	int32_t TestcasePauseStart(void);

	int32_t HandleTestcase(int sector,int seek);
private:
	TestcaseBase();
	TestcaseBase(const TestcaseBase& module) = delete;
	TestcaseBase& operator=(const TestcaseBase module) = delete;
	settingsgroup_class getGroupclass(QString group);
	void setupworkthread();
	void RegisterSignals();

	int32_t msgthreadloop();
	int32_t Handle_mfv();
	int32_t on_run(QSettings* settings);

	//static CRITICAL_SECTION g_winsec_settings;
	static TestcaseBasePtr _instance;
	QString runingcasename;
	DeviceBasePtrContainer mdevice_list;
	CaseItemBasePtrContain mcase_list;
	std::string ini_version;
	MessageFromViewContainer mmfvcontainer;
	MessageToViewContainer mmtvcontainer;
	INICommontagContainer minicommontagcontainer;
	ThreadworkControllerPtr  msgthread=nullptr;
};


