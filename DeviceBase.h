#pragma once
#include <string>
#include <memory>
#include <QSettings>
#include "VisaDriver.h"
#include "VictorDMMIDriver.h"
#include "workController.h"
#include "WorkThread.h"
#include "MessagePacket.h"
#include "GlobalConfig.h"
#include "Utility.h"
#define TESTCASE_VERSION_M	"0"
#define TESTCASE_VERSION_S	"0"
#define TESTCASE_VERSION_P	"1"
#define TESTCASE_VERSION TESTCASE_VERSION_M "." TESTCASE_VERSION_S "." TESTCASE_VERSION_P
#define SCPI_VERSION_1999	"1999.0"
enum class DeviceClass {
	DeviceClass_Unknow =0,//no special
	DeviceClass_DC,
	DeviceClass_DC_BatterySimulator,
	DeviceClass_digit_multimeter,
};
typedef struct DeviceStatus_t {
	bool connected=false;
	bool output = false;
	QString maxWVAStr="";
	int32_t voltage_mv;
}DeviceStatus;
typedef struct DeviceInfo_t {
	std::string identify;
	std::string networklabel;
	std::string interfaceid;
	DeviceClass deviceclass;
	DriverClass commuinterface;
}DeviceInfo;


#define IDENTIFY_STRING "identify"
#define NETWORKlabel_STRING "networklabel"
#define INTERFACEID_STRING "interfaceid"
#define ASRLBDPSF_STRING	"asrlbdpsf"
#define MAXPOWERWVA		"maxvA"
#define COMMUINTERFACE "commuinterface"
class DeviceBase;
typedef std::shared_ptr<DeviceBase> DeviceBasePtr;
typedef std::map<QString, std::string> DeviceBaseSettingMap;
typedef std::list<MessageFVBasePtr> CMDFVContainer;
using DeviceBasePtrContainer = std::vector<DeviceBasePtr>;
//Qobject 必须放第一个
class DeviceBase :public QObject // , public VisaDriver
{
	Q_OBJECT
	//typedef VisaDriver INTERIOR;
signals:
	void notifytoView(int, MessageTVBasePtr);
public:
	DeviceBase() = delete;
	~DeviceBase();
	static DeviceBasePtr get_instance(QSettings* settings,int);
	DeviceBase(int offset,std::string identify, std::string networklabel, std::string interfaceid, std::string arslconfg="", std::string maxva="", DriverClass driverclass= DriverClass::DriverSCPI);
	static QString GetVersion() {
		
		return QString("%1 [%2]").arg("DeviceBase").arg(TESTCASE_VERSION);
	};
	std::string GetIdentify();
	int32_t SetIdentifyCustomer(std::string value);
	std::string GetNetworklabel() {
		return networklabel;
	};
	std::string GetInterfaceId();
	bool InterfaceidSetable();
	int32_t SetInterfaceIdCustomer(std::string value);

	DeviceClass GetDeviceClass() {
		return mdevice_class;
	}
	DeviceInfo_t GetDeviceInfo();
	static bool checkingParam(QSettings* settings, DeviceBaseSettingMap &settingmap);
	int32_t threadloop();
	//事件
	int32_t connectasync(bool isconnect = true, std::string customerinterfaceid = {});
	int32_t disconnectasync();
	int32_t testactiveasync();
	//动作
	int32_t connectsync(std::string customerinterfaceid = {});
	int32_t disconnectsync();
	int32_t testactivesync();

	int32_t ioctrl(VisaDriverIoctrlBasePtr ptr=nullptr);
	int32_t GetdeviceStatus(DeviceStatus_t &st);

	Resourcecontainer FindResourcecontainer();
	QString GetDeviceSCPIVersion();
	bool isVirtualDevice() {
		return QString(interfaceidorig.c_str()).toUpper() == "VIRTUAL";
	}
private:
	int32_t Handlecmd();
	void setupworkthread();

	int32_t Readidentification(VisaDriverIoctrlBasePtr ptr);
	int32_t ReadSystemERRor(VisaDriverIoctrlBasePtr ptr);
	int32_t GetSystemERRorCount(VisaDriverIoctrlBasePtr ptr);
	int32_t SourceOutputState(VisaDriverIoctrlBasePtr ptr);
	int32_t SourceCurrentAmplitude(VisaDriverIoctrlBasePtr ptr);
	int32_t SourceVoltageAmplitude(VisaDriverIoctrlBasePtr ptr);
	int32_t ReadQuery(VisaDriverIoctrlBasePtr ptr);
	int32_t ReadQuery_1999(VisaDriverIoctrlBasePtr ptr);
	int32_t ReadQuery_1997(VisaDriverIoctrlBasePtr ptr);
	int32_t ReadQuery_victorDmmi(VisaDriverIoctrlBasePtr ptr);
	int32_t SystemLocalRemote(bool is_local=true);
private:
	std::string  identifyorig;
	std::string  identifycustomer;
	std::string networklabel;//存在多个网络标号
	std::string interfaceidorig;
	std::string interfaceidcustomer;
	std::string arslconfgstr;
	asrlconfg_t masrlconfg;
	//int32_t maxWVA = 8;
	int moffset_inlist;
	QString scpi_version;
	DeviceClass mdevice_class;
	DeviceStatus_t mdevicestatus;
	
	DriverClass mcommuinterface;
	NiDeviceDriverBasePtr interior_driver=nullptr;
	CMDFVContainer FVcontainer;
	ThreadworkControllerPtr msgthread = nullptr;
	static DeviceBasePtrContainer mstaticdeviceptrcontainer;
};

