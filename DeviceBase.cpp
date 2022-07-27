#include "DeviceBase.h"
//#include "DeviceBase.h"
#include "visa.h"
#include <winerror.h>
#include <QRegularExpression>
#include "NaturalLang.h"
#ifndef UI_DEBUG
#define USER_WATCH
#endif
DeviceBasePtrContainer DeviceBase::mstaticdeviceptrcontainer = {};

#define VISA_DEVICE_IOCTRL(xxx) \
		if (!isVirtualDevice())ret = interior_driver->ioctrl(xxx); \
		else { \
			qDebug("enter debug sleep"); \
			Utility::Sleep(500); \
			qDebug("leave debug sleep"); \
			goto ERROR_OUT; \
		}

DeviceBase::~DeviceBase()
{
	if(GlobalConfig_debugdevcieBase)qDebug(" ");
	Utility::OutputDebugPrintf("Enter ~DeviceBase %d\r\n",moffset_inlist);
	disconnectsync();
	Utility::OutputDebugPrintf("Leave ~DeviceBase %d\r\n", moffset_inlist);

}

DeviceBasePtr DeviceBase::get_instance(QSettings* settings,int offset)
{
	if (offset < mstaticdeviceptrcontainer.size()) {
		return mstaticdeviceptrcontainer[offset];
	}
	DeviceBaseSettingMap mmap;
	mmap.clear();
	if (checkingParam(settings, mmap) != true)return NULL;
	std::string maxva = "8";
	std::string initialmesa = "";
	if (mmap.find(QString(MAXPOWERWVA)) != mmap.end()) {
		maxva = mmap[QString(MAXPOWERWVA)];
	}
	
	if (mmap.find(QString(INITIALMESA)) != mmap.end()) {
		initialmesa = mmap[QString(INITIALMESA)];
	}
	DriverClass mdriverclass= DriverClass::DriverSCPI;
	if (mmap.find(QString(COMMUINTERFACE)) != mmap.end()) {
		QString comm = QString(mmap[QString(COMMUINTERFACE)].c_str());
		if (comm.toUpper()=="DMMI") {
			mdriverclass = DriverClass::DriverDMMIVictor;
		}
		else if(comm.toUpper() == "RELAYBMW"){
			mdriverclass = DriverClass::DriverRelayBMW;
		}
	}
	DeviceBasePtr ptr(new DeviceBase(offset,
		mmap[QString(IDENTIFY_STRING)],
		mmap[QString(NETWORKlabel_STRING)],
		mmap[QString(INTERFACEID_STRING)],
		mmap[QString(ASRLBDPSF_STRING)],
		maxva,
		initialmesa,
		mdriverclass));
	mstaticdeviceptrcontainer.push_back(ptr);
	return ptr;
}
bool strcasecmp(const char* s1, const char* s2) {
	return QString::compare(QString(s1), QString(s2), Qt::CaseInsensitive);

}
DeviceClass DeviceBase::CheckDeviceClassDC()
{
	auto default_type = DeviceClass::DeviceClass_DC;
	auto netid = QString(GetNetworklabel().c_str()).toUpper().split("/");
	foreach(auto str,netid) {
		if (str.size()>1&&(str.at(0) == 'I') ){
			default_type = DeviceClass::DeviceClass_DC_BatterySimulator;
			break;
		}
		if (str.size() > 1 && (str.at(0) == 'R')) {
			default_type = DeviceClass::DeviceClass_Relay_Switch;
			break;
		}
	}
	return default_type;
}
void DeviceBase::InitDeviceClassType()
{
	if (mcommuinterface == DriverClass::DriverSCPI) {
		interior_driver = NiDeviceDriverBasePtr(new VisaDriver);
		
		mdevice_class = CheckDeviceClassDC();
	}
	else if(mcommuinterface == DriverClass::DriverDMMIVictor) {
		interior_driver = NiDeviceDriverBasePtr(new VictorDMMIDriver);
		mdevice_class = DeviceClass::DeviceClass_Digit_Multimeter;
		interior_driver->SetCmdPostfix("\r\n");
	}
	else if (DriverClass::DriverRelayBMW== mcommuinterface) {
		interior_driver = NiDeviceDriverBasePtr(new RelayBMWDriver);
		mdevice_class = DeviceClass::DeviceClass_Relay_Switch;
	}
}
DeviceBase::DeviceBase(int offset,std::string iden, std::string net, std::string id, std::string confg,std::string maxva, std::string initial_mesa, DriverClass driverclass):
	moffset_inlist(offset),identifyorig(iden),networklabel(net),interfaceidorig(id), arslconfgstr(confg),
	initialmesa(initial_mesa),
	mcommuinterface(driverclass)
{
	int band, stopbits, databits;
	char parity[8], flow_ctr[8];
	if (GlobalConfig_debugdevcieBase)qDebug("dev[%d]arslconfgstr %s", moffset_inlist, arslconfgstr.c_str());
	
	int result = sscanf(arslconfgstr.c_str(), "%d/%d/%[^/]/%d/%s", &band, &databits, parity, &stopbits, flow_ctr);
	if (result == 5) {
		masrlconfg.baud_rate = band;
		masrlconfg.data_bits = databits;
		masrlconfg.stop_bits = stopbits;
		masrlconfg.parity = VI_ASRL_PAR_NONE;
		if (strcasecmp(parity, "NONE") == 0) {
			masrlconfg.parity = VI_ASRL_PAR_NONE;
		}
		else if (strcasecmp(parity, "ODD") == 0) {
			masrlconfg.parity = VI_ASRL_PAR_ODD;
		}
		else if (strcasecmp(parity, "EVEN") == 0) {
			masrlconfg.parity = VI_ASRL_PAR_EVEN;
		}
		else if (strcasecmp(parity, "MARK") == 0) {
			masrlconfg.parity = VI_ASRL_PAR_MARK;
		}
		else if (strcasecmp(parity, "SPACE") == 0) {
			masrlconfg.parity = VI_ASRL_PAR_SPACE;
		}
		masrlconfg.flow_control = VI_ASRL_FLOW_NONE;
		if (strcasecmp(flow_ctr, "NONE") == 0) {
			masrlconfg.flow_control = VI_ASRL_FLOW_NONE;
		}
		else if (strcasecmp(flow_ctr, "XON") == 0) {
			masrlconfg.flow_control = VI_ASRL_FLOW_XON_XOFF;
		}
		else if (strcasecmp(flow_ctr, "DTR") == 0) {
			masrlconfg.flow_control = VI_ASRL_FLOW_DTR_DSR;
		}
		else if (strcasecmp(flow_ctr, "RTS") == 0) {
			masrlconfg.flow_control = VI_ASRL_FLOW_RTS_CTS;
		}
	}
	else {
		if (strstr(interfaceidorig.c_str(), "ASRL")) {
			qCritical("dev[%d]arslconfgstr %s format fail result %d", moffset_inlist, arslconfgstr.c_str(), result);
		}
	}
	scpi_version = "";
	InitDeviceClassType();
	interior_driver->SetIndexInList(moffset_inlist);
	SetDeviceStatusOutput(false);
	SetDeviceStatusIsconnected(false);
	mdevicestatus.maxWVAStr = maxva.size() > 0 ? QString(maxva.c_str()) : QString("8");
	DumpDebug();
	interfaceidcustomer.clear();
	FVcontainer.clear();
	setupworkthread();
}
std::string DeviceBase::GetIdentify()
{
	std::string tmp = identifyorig;
	if ((stricmp(identifyorig.c_str(), "auto") == 0)
		&&
		(identifycustomer.size() > 0)) {
		tmp = identifycustomer;
	}
	if (GlobalConfig_debugdevcieBase)qDebug("index %d [%s] [%s]", moffset_inlist, tmp.c_str(), identifycustomer.c_str());
	return tmp;
}
std::string DeviceBase::GetInterfaceId()
{
	std::string tmp = interfaceidorig;
	if ((stricmp(interfaceidorig.c_str(), "auto") == 0)
		&& 
		(interfaceidcustomer.size()>0)) {
		tmp = interfaceidcustomer;
	}
	if (GlobalConfig_debugdevcieBase)qDebug("index %d [%s] [%s]",moffset_inlist,tmp.c_str(), interfaceidcustomer.c_str());
	return tmp;
}
bool DeviceBase::InterfaceidSetable()
{
	bool ret = false;
	if (!isVirtualDevice()) {
		if (stricmp(interfaceidorig.c_str(), "auto") == 0 && interfaceidcustomer.size() == 0) {
			ret = true;
		}
	}
	if (GlobalConfig_debugdevcieBase)qDebug("index %d ret %d",moffset_inlist,ret);
	return ret;
}
int32_t DeviceBase::SetInterfaceIdCustomer(std::string value)
{
	if (GlobalConfig_debugdevcieBase)qDebug(" value %s", value.c_str());
	int ret = -1;
	if (stricmp(interfaceidorig.c_str(), "auto") == 0){
		interfaceidcustomer = value;
		ret = 0;
	}
	return ret;
}
int32_t DeviceBase::SetIdentifyCustomer(std::string value)
{
	if (GlobalConfig_debugdevcieBase)qDebug(" value %s", value.c_str());
	int ret = -1;
	if (stricmp(identifyorig.c_str(), "auto") == 0) {
		identifycustomer = value;
		ret = 0;
	}
	return ret;
}
DeviceInfo_t DeviceBase::GetDeviceInfo()
{
	DeviceInfo_t minfo;
	minfo.identify=GetIdentify();
	minfo.networklabel=GetNetworklabel();
	minfo.interfaceid=GetInterfaceId();
	minfo.deviceclass=mdevice_class;
	minfo.commuinterface = mcommuinterface;
	return minfo;
}
bool DeviceBase::checkingParam(QSettings* settings, DeviceBaseSettingMap& settingmap)
{
	bool ret = false;

	foreach(auto key ,settings->allKeys())
	{
		bool checkok = true;
		do {
			
			auto value = settings->value(key).toString();
			if(GlobalConfig_debugtestcaseini)qDebug("key %s ==%s\n", key.toStdString().c_str(), value.toStdString().c_str());
#if 1
			if (key== IDENTIFY_STRING ||key==NETWORKlabel_STRING||key==INTERFACEID_STRING||key== MAXPOWERWVA) {
				if (value.size() == 0) { checkok = false; break; }
				settingmap[key] = value.toStdString();
			}else 
			if (key == ASRLBDPSF_STRING) {
				int band, stopbits, databits;
				char parity[8] = { 0 },flow_ctr[8] = { 0 };
				int result = sscanf(value.toStdString().c_str(),"%d/%d/%[^/]/%d/%s",&band,&databits,parity,&stopbits, flow_ctr);
				if(result!=5) { 
					checkok = false; 
					qCritical("param %s result %d\n", value.toStdString().c_str(), result);
					break; 
				}
				settingmap[key] = value.toStdString();
			}
			else {
				settingmap[key] = value.toStdString();
			}

#endif
		} while (0);
		if(settingmap.size()>=3 && checkok)ret = checkok;
		if (!checkok)break;
	}

	if (ret != true) {
		qCritical("checkingParam fail");
	}
	return ret;
}
//缓存cmd
int32_t DeviceBase::connectasync(bool isconnect, std::string customerinterfaceid)
{
	int ret = 0;

	MessageFVDeviceConnectDis* payload = new MessageFVDeviceConnectDis;
	payload->isconnect = isconnect;
	if (InterfaceidSetable() && customerinterfaceid.size() > 0) {

		payload->customerinterfaceid = customerinterfaceid;
	}
	MessageFVBasePtr ptr(payload);
	FVcontainer.push_back(ptr);
	WORKTHREADWAIT(msgthread).notify_all();
	return ret;
}
int32_t DeviceBase::testactiveasync()
{
	
	int ret = 0;
	MessageFVBase* payload = new MessageFVBase;
	payload->cmd = MessageFromView::TestactiveDevice;
	MessageFVBasePtr ptr(payload);
	FVcontainer.push_back(ptr);
	WORKTHREADWAIT(msgthread).notify_all();
	return ret;
}

int32_t DeviceBase::testactivesync()
{
	if (mdevice_class <= DeviceClass::DeviceClass_DC_BatterySimulator) {
		auto cmd = new DeviceDriverOutputState;
		VisaDriverIoctrlBasePtr ptr(cmd);
		return ioctrl(ptr);
	}
	else if (mdevice_class == DeviceClass::DeviceClass_Digit_Multimeter) {
		auto cmd = new DeviceDriverReadQuery;
		VisaDriverIoctrlBasePtr ptr(cmd);
		auto type = GetNetworklabel().front();
		if (type == 'V') {
			cmd->mMeasfunc = DeviceDriverReadQuery::QueryMeasFunc::MeasDCV;
		}
		else if (type == 'I') {
			cmd->mMeasfunc = DeviceDriverReadQuery::QueryMeasFunc::MeasDCI;
		}
		return ioctrl(ptr);
	}
	else if (mdevice_class == DeviceClass::DeviceClass_Relay_Switch) {
		auto cmd = new DeviceDriverRelayChannelRW;
		VisaDriverIoctrlBasePtr ptr(cmd);
		return ioctrl(ptr);
	}
	return -ERROR_UNSUPPORTED_TYPE;
}
int32_t DeviceBase::InitialMese(QString qinitialmesa)
{
	int ret = 0;
	if (DriverClass::DriverSCPI == mcommuinterface) {
		SystemLocalRemote(false);
	}
	if (DeviceClass::DeviceClass_DC_BatterySimulator == mdevice_class) {
		auto msg = new DeviceDriverWorkFunction;
		msg->is_read = false;
		msg->wfunctions = DeviceWorkFunc::POWer;
		auto ptr = VisaDriverIoctrlBasePtr(msg);
		EntryFuction(ptr);
	}
	GetDeviceSCPIVersion();
	if (mdevice_class<= DeviceClass::DeviceClass_DC_BatterySimulator) {
		//initial 
		if (qinitialmesa.size()!=0) {
			QString output = "";
			auto ptr = NaturalLang::translation_slash_smart(qinitialmesa.toStdString(), output, caseitem_class::Precondition);
			NetworkLabelPreconditionBase* info = dynamic_cast<NetworkLabelPreconditionBase*>(ptr.get());
			if (info == nullptr) {
				ret = -ERROR_INVALID_PARAMETER;
				goto ERR_OUT;
			}
			ret = CaseItemBase::FunctionSetVoltageOut(moffset_inlist, info);
		}
	}
	if (DeviceClass::DeviceClass_Relay_Switch== mdevice_class) {
		if (qinitialmesa.size() != 0) {
			QString output = "";
			auto ptr = NaturalLang::translation_slash_smart(qinitialmesa.toStdString(), output, caseitem_class::Precondition);
			NetworkLabelPreconditionRelay* info = dynamic_cast<NetworkLabelPreconditionRelay*>(ptr.get());
			if (info == nullptr) {
				ret = -ERROR_INVALID_PARAMETER;
				goto ERR_OUT;
			}
			ret = CaseItemBase::FunctionRelayRW(moffset_inlist, info);
		}
	}

ERR_OUT:
	return ret;
}
int32_t DeviceBase::connectsync(std::string customerinterfaceid)
{
	if (GlobalConfig_debugdevcieBase)qDebug("index %d customerinterfaceid [%s]",moffset_inlist, customerinterfaceid.c_str());
	int ret = 0;
	std::string tmp = GetInterfaceId();
	{

		if (customerinterfaceid.size() > 0 ) {
			if (InterfaceidSetable()) {
				tmp = customerinterfaceid;
			}
			else {
				qCritical("device had explicit interfaceid %s can't set to %s",
					tmp.c_str(), customerinterfaceid.c_str());
				ret = -ERROR_INVALID_PARAMETER;
				goto ERROR_OUT;
			}
		}
		if(!isVirtualDevice())ret = interior_driver->Driveropen(tmp);
		else {
			Utility::Sleep(1 * 1000);//for debug 
		}
	}
	if (ret != 0/*&& mcommuinterface != DriverClass::DriverDMMIVictor*/) {
		qCritical("index %d interfaceid [%s] open fail", moffset_inlist, tmp.c_str());
		goto ERROR_OUT;
	}
	if (ret == 0 && (arslconfgstr.size() > 0 || QString(GetInterfaceId().c_str()).toUpper().contains("ASRL")))
	{
		if (!isVirtualDevice())ret = interior_driver->Driversetattribute(masrlconfg);
	}
	InitialMese(QString(initialmesa.c_str()));
	//testconnect
	ret = testactivesync();	
	if (ret ==0) {
		SetDeviceStatusIsconnected(true);
	}
	else {
		SetDeviceStatusIsconnected(false);
		goto ERROR_OUT;
	}
	if (QString(GetIdentify().c_str()).toUpper() == "AUTO") {
		auto msg = new VisaDriverIoctrlBase;
		msg->cmd = VisaDriverIoctrl::ReadIdentification;
		auto ptr = VisaDriverIoctrlBasePtr(msg);
		Readidentification(ptr);
	}
	if (mdevicestatus.connected) {
		SetInterfaceIdCustomer(tmp);
	}

	setupwatchthread();
ERROR_OUT:
	if (!mdevicestatus.connected) {
		if (!isVirtualDevice())ret = interior_driver->Driverclose();
		auto msg = new MessageTVLogWidgetUpdate;
		msg->msg = QStringLiteral("设备 %1 连接失败").arg(GetNetworklabel().c_str());
		MessageTVBasePtr ptr(msg);
		emit notifytoView(int(msg->GetCmd()), ptr);
	}
	return ret;
}
int32_t DeviceBase::disconnectasync()
{
	return connectasync(false);
}
void DeviceBase::SetDeviceStatusIsconnected(bool val) 
{
	mdevicestatus.connected = val;
}
int32_t DeviceBase::disconnectsync()
{
	if (GlobalConfig_debugdevcieBase)qDebug("index %d ", moffset_inlist);
	int ret = 0;
	if (!mdevicestatus.connected)return ret;
	if(mdevice_class <= DeviceClass::DeviceClass_DC_BatterySimulator) {
		//先关闭输出
		auto mptrsv = VisaDriverIoctrlBasePtr(new DeviceDriverSourceVoltage);
		DeviceDriverSourceVoltage* upper = dynamic_cast<DeviceDriverSourceVoltage*>(mptrsv.get());
		upper->is_read = false;
		upper->voltage_mv = 0;
		ret = SourceVoltageAmplitude(mptrsv);
	
	}
	else if (mdevice_class== DeviceClass::DeviceClass_Relay_Switch) {
		auto mptrsv = VisaDriverIoctrlBasePtr(new DeviceDriverRelayChannelRW);
		DeviceDriverRelayChannelRW* upper = dynamic_cast<DeviceDriverRelayChannelRW*>(mptrsv.get());
		upper->is_read = false;
		upper->channelmask = 0xff;
		upper->channelvalue = 0;
		ret = RelayChannel(mptrsv);
		//qCritical("~ here crash？");//~ with qCritical crash
	}
	if (mcommuinterface==DriverClass::DriverSCPI) {
		ret = SystemLocalRemote(true);
	}
	if (!isVirtualDevice()) ret= interior_driver->Driverclose();
	if (ret == 0) {
		SetDeviceStatusIsconnected(false);
	}
	else {
		SetDeviceStatusIsconnected(true);
	}
	//clean customerinterfaceid
	SetInterfaceIdCustomer("");
	return ret;
}

int32_t DeviceBase::GetdeviceStatus(DeviceStatus_t& st)
{
	int ret = 0;
	st = mdevicestatus;
	return ret;
}
Resourcecontainer DeviceBase::FindResourcecontainer()
{
	Resourcecontainer tmp;
	int ret = interior_driver->FindDeviceResource(tmp);
	auto msg = new MessageTVHardWareDeviceRes;
	msg->index = moffset_inlist;
	msg->is_success = (ret == 0);
	if (msg->is_success) {
		msg->res = tmp;
	}
	MessageTVBasePtr ptr(msg);
	emit notifytoView(int(msg->GetCmd()), ptr);
	return tmp;
}

int32_t DeviceBase::Handlecmd()
{
	int ret = 0;
	if (FVcontainer.size() == 0)return ret;
	
	auto payload= FVcontainer.begin();
	MessageFromView cmd = payload->get()->GetCmd();
	switch (cmd) {
	case MessageFromView::ConnectDisDevice:
	{
		auto devp = payload->get();
		MessageFVDeviceConnectDis* dev = dynamic_cast<MessageFVDeviceConnectDis*>(devp);
		if (dev->isconnect) {
			ret = connectsync(dev->customerinterfaceid);
		}
		else {
			ret = disconnectsync();
		}
		{
			auto msg = new MessageTVDeviceUpdate;
			msg->index = moffset_inlist;
			msg->icon = DeviceStatusIcon::Connect;
			MessageTVBasePtr ptr(msg);
			emit notifytoView(int(msg->GetCmd()), ptr);
		}
	}
	break;
	case MessageFromView::TestactiveDevice:
	{
		ret = testactivesync();
		{
			auto msg = new MessageTVDeviceUpdate;
			msg->index = moffset_inlist;
			msg->icon = DeviceStatusIcon::TestActive;
			MessageTVBasePtr ptr(msg);
			emit notifytoView(int(msg->GetCmd()), ptr);
		}
		{
			auto msg = new MessageTVLogWidgetUpdate;
			msg->index = moffset_inlist;
			QString status = ret == 0 ? QStringLiteral("正常") : QStringLiteral("失败");
			msg->msg = QStringLiteral("设备 %2 连接 %1").arg(status).arg(QString(GetNetworklabel().c_str()));
			MessageTVBasePtr ptr(msg);
			emit notifytoView(int(msg->GetCmd()), ptr);
		}
	}
	break;
	default:
		qCritical("cmd %d unknown", cmd);
		break;
	}
ERR_OUT:
	FVcontainer.pop_front();
	return ret;
}
void DeviceBase::setupworkthread()
{
	if (msgthread == nullptr) {
		ThreadworkControllerPtr ptr(new ThreadworkController(std::bind(&DeviceBase::threadloopMsg, this)));
		msgthread = ptr;
	}
}
void DeviceBase::SetDeviceStatusOutput(bool val)
{
	//qDebug("set output devicestatus %d",val);
	mdevicestatus.output = val;
	if (devicestatusthread) {
		WORKTHREADWAIT(devicestatusthread).notify_all();
	}
}
void DeviceBase::DumpDebug()
{
	if (GlobalConfig_debugdevcieBase) 
	{
		qDebug("device %d idn %s:",moffset_inlist,GetIdentify().c_str());
		qDebug("\tmdevice_class %d mcommuinterface %d",mdevice_class,mcommuinterface);
	}
}
void DeviceBase::setupwatchthread()
{
#ifdef USER_WATCH
	if (mdevice_class> DeviceClass::DeviceClass_Digit_Multimeter) {
		//device not nead watch
		return;
	}
	if (devicestatusthread == nullptr) {
		ThreadworkControllerPtr ptr(new ThreadworkController(std::bind(&DeviceBase::threadloopStatus, this)));
		devicestatusthread = ptr;
	}
	if (mcommuinterface==DriverClass::DriverDMMIVictor) {
		WORKTHREADWAIT(devicestatusthread).notify_all();
	}
#endif
}
int32_t DeviceBase::threadloopStatus()
{
	int ret = 0;
	QMutexLocker locker(&WORKTHREADMUTEX(devicestatusthread));
	unsigned long shortsleeptime = 800;
	unsigned long sleeptime = shortsleeptime;
	bool hadclearmsg = false;
	do {
		auto result = WORKTHREADWAIT(devicestatusthread).wait(&WORKTHREADMUTEX(devicestatusthread), 3 * 1000);
		//qDebug("wait timtout %d",result);
		if (GlobalConfig_debugdevcieBase)qDebug("devicestatusthread leave wait ");
		//qDebug("mdevicestatus.output %d lastmdevicestatus_output %d", mdevicestatus.output, lastmdevicestatus_output);
		while( (devicestatusthread->isthreadkeeprun()) && 
			(mdevicestatus.output == true || mcommuinterface == DriverClass::DriverDMMIVictor))
		{
			{
				Utility::Sleep(sleeptime);
				auto mptrrq = VisaDriverIoctrlBasePtr(new DeviceDriverReadQuery);
				DeviceDriverReadQuery* upper = dynamic_cast<DeviceDriverReadQuery*>(mptrrq.get());
				if (mcommuinterface == DriverClass::DriverDMMIVictor) {
						upper->mMeasfunc = DeviceDriverReadQuery::QueryMeasFunc::MeasDCV;
				}
				ret = ioctrl(mptrrq);
				if (ret != 0) {
					qCritical("IOCTRL DeviceDriverReadQuery fail");
					ret = -ERROR_DEVICE_UNREACHABLE;
					goto ERROR_OUT;
				}
				else {
					hadclearmsg = false;
					auto msg = new MessageTVDeviceUpdate;
					msg->index = moffset_inlist;
					msg->icon = DeviceStatusIcon::Voltage_Current;
					msg->payload = upper->ShowStatus();
					MessageTVBasePtr ptr(msg);
					emit notifytoView(int(msg->GetCmd()), ptr);
					sleeptime = shortsleeptime;
				}
			}

		}//end while
		//清除buf
		if(!hadclearmsg) {
			qDebug("notice null");
				auto msg = new MessageTVDeviceUpdate;
				msg->index = moffset_inlist;
				msg->icon = DeviceStatusIcon::Voltage_Current;
				msg->payload = " ";
				MessageTVBasePtr ptr(msg);
				emit notifytoView(int(msg->GetCmd()), ptr);
				hadclearmsg = true;
		}
	ERROR_OUT:
		sleeptime = 3 * 1000;
	}while(devicestatusthread->isthreadkeeprun());
	if (GlobalConfig_debugdevcieBase)qDebug("devicestatusthread leave");
	return ret;
}
#if 1
int32_t DeviceBase::threadloopMsg()
{
	QMutexLocker locker(&WORKTHREADMUTEX(msgthread));

	int ret = 0;
	do {
		if(GlobalConfig_debugthread)qDebug(" enter wait");
		if (FVcontainer.empty() ) {
			WORKTHREADWAIT(msgthread).wait(&WORKTHREADMUTEX(msgthread));
		}
		if(GlobalConfig_debugthread)qDebug(" leave wait ");
		while (msgthread->isthreadkeeprun() && !FVcontainer.empty()) {
			Handlecmd();		
		}
	} while (msgthread->isthreadkeeprun());

	return ret;
}

#endif
int32_t DeviceBase::ioctrl(VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	QMutexLocker locker(&mdevicemutex);
#if 0 //有一些clean 命令完全不需要参数
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
#endif
	VisaDriverIoctrl cmd = ptr->GetCmd();
	switch (cmd)
	{
	case VisaDriverIoctrl::IoRead:
	case VisaDriverIoctrl::IoWrite:
		if (GlobalConfig_debugdevcieBase)qDebug("command %s",ptr->commond.c_str());
		ret = 0;
		if (!isVirtualDevice())ret = interior_driver->ioctrl(ptr);
		break;
	case VisaDriverIoctrl::ReadSystemError:
		ret = ReadSystemERRor(ptr);
		break;
	case VisaDriverIoctrl::ReadSystemErrorCount:
		ret = GetSystemERRorCount(ptr);
		break;
	case VisaDriverIoctrl::ReadIdentification:
		ret = Readidentification(ptr);
		break;
	case VisaDriverIoctrl::OutputState:
		ret = SourceOutputState(ptr);
		break;
	case VisaDriverIoctrl::SourceCurrentLimit:
		ret = SourceCurrentAmplitude(ptr);
		break;
	case VisaDriverIoctrl::SourceVoltage:
		ret = SourceVoltageAmplitude(ptr);
		break;
	case VisaDriverIoctrl::ReadQuery:
		ret = ReadQuery(ptr);
		break;
	case VisaDriverIoctrl::RelayChannelRW:
		ret = RelayChannel(ptr);
		break;
	default:
		qCritical("cmd %d unsupport",cmd);
		ret = -ERROR_INVALID_PARAMETER;
		break;
	}
ERROR_OUT:
	if (ret == 0) {
		if(GlobalConfig_debugdevcieBase)qDebug("ret %d cmd %d", ret,cmd);
	}
	else {
		qCritical("cmd %d ret 0x%x  [%s]", cmd, ret, Utility::GetWinErrorText(ret).toStdString().c_str());
	}
	return ret;
}
int32_t DeviceBase::SourceOutputState(VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	std::string command_read = ":OUTPut?";
		
	if (GlobalConfig_debugdevcieBase)qDebug(" ");
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		qCritical("Null param");
		goto ERROR_OUT;
	}
	if (mcommuinterface == DriverClass::DriverDMMIVictor) {
		qInfo(" driver %d unsupport", mcommuinterface);
		ret = 0;
		goto ERROR_OUT;
	}
	DeviceDriverOutputState* upper_arg = dynamic_cast<DeviceDriverOutputState*>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (upper_arg->channel != -1) {
		//hand channel
		command_read =(QString("INSTrument:NSELect %1;SOURce:CHANnel:OUTPut?").arg(upper_arg->channel).toStdString());
	}
	if (upper_arg->is_read) {
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = command_read;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			//:OUTP ON 是否它的返回一定是字符？
			{
				upper_arg->result = mptr->result;
				QString input_str(mptr->result.c_str());
				bool onoff = false;
				if (input_str.toUpper().contains("ON") || input_str.toUpper().contains("1")) {
					onoff = true;
				}
#if 0
				//QRegularExpression re(":(?<command>\\w+) (?<state>\\w+)");
				QRegularExpression re("(?<statenum>\\d+<statestr>\\w*)");
				QRegularExpressionMatch match = re.match(input_str);
				if (!match.hasMatch()) {
					ret = -ERROR_DEVICE_HARDWARE_ERROR;
					goto ERROR_OUT;
				}
#endif
				/*
				if (match.captured("command").compare("OUTP", Qt::CaseInsensitive) != 0) {
					ret = -ERROR_DEVICE_HARDWARE_ERROR;
					goto ERROR_OUT;
				}
				*/
				upper_arg->onoff = onoff;
			}
		}
	}
	else {
			VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlWrite);
			//开要强开，关自身通道
			std::string command = "";

			if (upper_arg->channel != -1) {
				//hand channel
				if (upper_arg->onoff) {
					command = ":OUTPut 1";
				}
				command.append(QString(";INSTrument:NSELect %1;SOURce:CHANnel:OUTPut %2").arg(upper_arg->channel).arg(upper_arg->onoff).toStdString());
			}
			else {
				command = ":OUTPut";
				if (upper_arg->onoff) {
					command.append(" 1");
				}
				else {
					command.append(" 0");
				}
			}
			mptr->commond = command;
			VISA_DEVICE_IOCTRL(mptr)
			ptr->commond = mptr->commond;

			if ( (upper_arg->onoff==true) && 
				(ret==0)) {
				SetDeviceStatusOutput(true);
			}
			else {
				SetDeviceStatusOutput(false);
			}
			if ((GetDeviceSCPIVersion() < SCPI_VERSION_1999)) {
				//E3640A 特性
				Utility::Sleep(800);
			}
		}
	
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::SourceCurrentAmplitude(VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	std::string command="";
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (mcommuinterface == DriverClass::DriverDMMIVictor) {
		qInfo("dev[%d] driver %d unsupport",moffset_inlist, mcommuinterface);
		ret = 0;
		goto ERROR_OUT;
	}
	DeviceDriverSourceCurrentLimit* upper_arg = dynamic_cast<DeviceDriverSourceCurrentLimit*>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (upper_arg->channel != -1) {
		//hand channel
		command = (QString("INSTrument:NSELect %1;").arg(upper_arg->channel).toStdString());
	}
	if (upper_arg->is_read) 
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		command.append(":SOURce");

		command.append(":CURRent?");
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0 ) {
			upper_arg->result = mptr->result;
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			upper_arg->current_ma = int(atof(mptr->result.c_str()) * 1000);
		}
	}
	else {
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlWrite);
		command.append(":SOURce");

		command.append(":CURRent ");
		float current_A = (float)upper_arg->current_ma / 1000;
		command.append(std::to_string(current_A));
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		upper_arg->result = mptr->result;
		if (GetDeviceSCPIVersion() < SCPI_VERSION_1999) {
			//合理报错
			if (GlobalConfig_debugdevcieBase)qDebug("less SCPI_VERSION_1999 force sleep");
			Utility::Sleep(1000);
		}
	}
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::SourceVoltageAmplitude(VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	std::string command="";
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (mcommuinterface == DriverClass::DriverDMMIVictor) {
		qInfo("dev[%d] driver %d unsupport", moffset_inlist, mcommuinterface);
		ret = 0;
		goto ERROR_OUT;
	}
	DeviceDriverSourceVoltage* upper_arg = dynamic_cast<DeviceDriverSourceVoltage*>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (upper_arg->channel!=-1) {
		//hand channel
		command.append(QString("INSTrument:NSELect %1;").arg(upper_arg->channel).toStdString());
	}
	if (upper_arg->is_read)
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		upper_arg->voltage_mv = 0;
		command.append(":SOURce");
		command.append(":VOLTage?");
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0) {
			upper_arg->result = mptr->result;
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			upper_arg->voltage_mv = int(atof(mptr->result.c_str()) * 1000);
			//qDebug("str %s %d", mptr->result.c_str(), upper_arg->voltage_mv);
		}
	}
	else {
			VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlWrite);
			command.append(":SOURce");
			command.append(":VOLTage ");
			float voltage_V = (float)upper_arg->voltage_mv / 1000;
			command.append(std::to_string(voltage_V));
			mptr->commond = command;
			VISA_DEVICE_IOCTRL(mptr)

			ptr->commond = mptr->commond;
			upper_arg->result = mptr->result;
			if(voltage_V==0) {
				auto mptros = VisaDriverIoctrlBasePtr(new DeviceDriverOutputState);
				DeviceDriverOutputState* upper = dynamic_cast<DeviceDriverOutputState*>(mptros.get());
				upper->is_read = false;
				upper->onoff = false;
				upper->channel = upper_arg->channel;
				SourceOutputState(mptros);
			}
	}
ERROR_OUT:
	return ret;
}

int32_t DeviceBase::Readidentification(VisaDriverIoctrlBasePtr ptr)
{
	std::string command = "*IDN?";
	int ret = 0;
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (mcommuinterface==DriverClass::DriverDMMIVictor) {
		qInfo("dev[%d] driver %d unsupport", moffset_inlist, mcommuinterface);
		ret = 0;
		goto ERROR_OUT;
	}
	//不应该改orig ptr
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = command;	
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			ptr->result = mptr->result;
			SetIdentifyCustomer(mptr->result);
		}
	}
ERROR_OUT:
	return ret;
}

int32_t DeviceBase::ReadSystemERRor(VisaDriverIoctrlBasePtr ptr)
{
	std::string command = ":SYSTem:ERRor?";
	int ret = 0;
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
//无私有packet
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}

			ptr->result = mptr->result;
		}
	}
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::GetSystemERRorCount(VisaDriverIoctrlBasePtr ptr)
{
	std::string command = ":SYSTem:ERRor:COUNt?";
	int ret = 0;
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (GetDeviceSCPIVersion() < SCPI_VERSION_1999) {
		qCritical(" %s unsupport this", GetDeviceSCPIVersion().toStdString().c_str());
	}
	DeviceDriverReadSystemErrorCount* upper_arg = dynamic_cast<DeviceDriverReadSystemErrorCount *>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			upper_arg->result = mptr->result;
			upper_arg->result_cnt = atoi(mptr->result.c_str());
		}
	}
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::EntryFuction(VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	bool need_set=false;
	QString command = ":ENTRy:FUNCtion";
	if (mdevice_class != DeviceClass::DeviceClass_DC_BatterySimulator) {
		qInfo(" Deviceclass %d unsupport", mdevice_class);
		ret = 0;
		goto ERROR_OUT;
	}
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	DeviceDriverWorkFunction* upper_arg = dynamic_cast<DeviceDriverWorkFunction*>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	//read first
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = (command+"?").toStdString();
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			upper_arg->result = mptr->result;
			auto func_value = CheckDeviceWorkFunc(mptr->result);
			if (upper_arg->wfunctions!=DeviceWorkFunc::ENTRy && func_value!= upper_arg->wfunctions) {
				need_set = true;
			}
		}
	}
	if (need_set && upper_arg->wfunctions!=DeviceWorkFunc::ENTRy) {
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlWrite);
		QString tmp = upper_arg->wfunctions == DeviceWorkFunc::POWer ? "POW" :
			upper_arg->wfunctions == DeviceWorkFunc::TEST ? "TEST" :
			upper_arg->wfunctions == DeviceWorkFunc::SIMulator ? "SIM" :
			"";
		mptr->commond = (command + " " + tmp).toStdString();
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond.append(mptr->commond);
		if (ret == 0) {
			upper_arg->result.append(mptr->result);
		}
	}
	
ERROR_OUT:
	return ret;
}
DeviceWorkFunc DeviceBase::CheckDeviceWorkFunc(QString input)
{
	auto default_workfunc = DeviceWorkFunc::ENTRy;
	if (input.left(QString("ENTR").size()) == "ENTR") {
		goto ERROR_OUT;
	}
	if (input.left(QString("POW").size())=="POW") {
		default_workfunc = DeviceWorkFunc::POWer;
		goto ERROR_OUT;
	}
	if (input.left(QString("TEST").size()) == "TEST") {
		default_workfunc = DeviceWorkFunc::TEST;
		goto ERROR_OUT;
	}
	if (input.left(QString("SIM").size()) == "SIM") {
		default_workfunc = DeviceWorkFunc::SIMulator;
		goto ERROR_OUT;
	}
ERROR_OUT:
	return default_workfunc;
}
DeviceWorkFunc DeviceBase::CheckDeviceWorkFunc(std::string input)
{
	return CheckDeviceWorkFunc(QString(input.c_str()));
}
int32_t DeviceBase::ReadQuery(VisaDriverIoctrlBasePtr ptr)
{
	if(mcommuinterface == DriverClass::DriverSCPI){
		if (GetDeviceSCPIVersion() < SCPI_VERSION_1999) {
			return ReadQuery_1997(ptr);
		}
		else {
			return ReadQuery_1999(ptr);
		}
	}
	else if (mcommuinterface == DriverClass::DriverDMMIVictor) {
		return ReadQuery_victorDmmi(ptr);
	}
	else {
		return -ERROR_INVALID_PARAMETER;
	}

}
int32_t DeviceBase::ReadQuery_victorDmmi(VisaDriverIoctrlBasePtr ptr) 
{

	int ret = 0;
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	DeviceDriverReadQuery* upper_arg = dynamic_cast<DeviceDriverReadQuery*>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	{
		if (upper_arg->mMeasfunc != DeviceDriverReadQuery::QueryMeasFunc::MeasDCV &&
			upper_arg->mMeasfunc != DeviceDriverReadQuery::QueryMeasFunc::MeasDCI) {
			qCritical("mMeasfunc %d software unsupport", upper_arg->mMeasfunc);
		}
		bool need_ins = false;
		VDMMeasSpanV spav = VDMMeasSpanV::V50;
		VDMMeasSpanI spai = VDMMeasSpanI::A5A;
		VDMeasRate rate = VDMeasRate::Slow;
		int spa = int(spav);
		if (upper_arg->mMeasfunc == DeviceDriverReadQuery::QueryMeasFunc::MeasDCI) {
			spa = int(spai);
		}
		//先读模式，没有再设置
		{
			QString command = QString::asprintf("#*RS?");
			VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
			mptr->commond = command.toStdString();
			VISA_DEVICE_IOCTRL(mptr)
			ptr->commond = mptr->commond;
			ptr->result = mptr->result;
			if (ret == -ERROR_TIMEOUT) {
				need_ins = true;
			}
			if (ret == 0) {
				//#*RS030
				auto raw_result = QString(mptr->result.c_str());
				QRegularExpression rexpre("#*RS(?<tfunc>\\d{1})(?<tspan>\\d{1})(?<trate>\\d{1})");
				QRegularExpressionMatch match = rexpre.match(raw_result);
				if (match.hasMatch()) {
					do {
						auto mtfunc = match.captured("tfunc").toInt();
						if (int(upper_arg->mMeasfunc) != mtfunc) {
							need_ins = true;
							break;
						}
						if (spa != match.captured("tspan").toInt()) {
							need_ins = true;
							break;
						}
						if (int(rate) != match.captured("trate").toInt()) {
							need_ins = true;
							break;
						}
					} while (0);
				}
				else {
					qCritical("value %s match fail", raw_result.toLocal8Bit().constData());
					ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
					goto ERROR_OUT;
				}

			}


		}
		//先设置模式，如果有ack 需要sleep下等待切换。再rd
		if (need_ins) {
			//可能要升档，还有用
			QString command = QString::asprintf("#*INS%d%d%d", upper_arg->mMeasfunc, spa, VDMeasRate::Slow);
			VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
			mptr->commond = command.toStdString();
			VISA_DEVICE_IOCTRL(mptr)
			ptr->commond.append(mptr->commond);
			ptr->result.append(mptr->result);
			if (ret == 0) {
				Utility::Sleep(3 * 1000);
			}
			else if (ret != 0) {
				goto ERROR_OUT;
			}

		}
		{
			QString tmp = "#*RD?";
			VisaDriverIoctrlBasePtr tmpptr(new VisaDriverIoctrlRead);
			tmpptr->commond = tmp.toStdString();
			VISA_DEVICE_IOCTRL(tmpptr)
			ptr->commond.append(tmpptr->commond);
			if (ret == 0) {
				if (tmpptr->result.size() <= 0) {
					ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
					goto ERROR_OUT;
				}
				ptr->result.append(tmpptr->result);
				{
					QString raw_result = QString(tmpptr->result.c_str());
					//#*RD+FFFFFF+0000.C 超出量程
					//#*RD+3.9662+0000.C
					QRegularExpression rexpre("#*RD(?<value_get>[-+]?[0-9F]+\\.?[0-9F]*)\\+0000.C");
					QRegularExpressionMatch match = rexpre.match(raw_result);
					if (match.hasMatch()) {
						//qDebug("value_get %s", match.captured("value_get").toStdString().c_str());
						if (match.captured("value_get").toUpper() == "+FFFFFF") {
							qCritical("Out of rang");
							ret = -ERROR_RANGE_NOT_FOUND;
							goto ERROR_OUT;
						}
						auto vvalue = match.captured("value_get").toDouble();
						if (upper_arg->mMeasfunc == DeviceDriverReadQuery::QueryMeasFunc::MeasDCI) {
							upper_arg->current_ma = vvalue * 1000;
						}
						if (upper_arg->mMeasfunc == DeviceDriverReadQuery::QueryMeasFunc::MeasDCV) {
							upper_arg->voltage_mv = vvalue * 1000;
						}
					}
					else {
						qCritical("value %s match fail", raw_result.toLocal8Bit().constData());
						ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
						goto ERROR_OUT;
					}
				}
			}

		}
	}
ERROR_OUT:
		return ret;
}
int32_t DeviceBase::SystemLocalRemote(bool is_local)
{
	int ret = 0;
	if (mcommuinterface == DriverClass::DriverDMMIVictor) {
		qInfo(" driver %d unsupport", mcommuinterface);
		ret = 0;
		goto ERROR_OUT;
	}
	{
		std::string command = ":SYSTem:";
		if (is_local) {
			command.append("LOCal");
		}
		else {
			command.append("REMote");
		}
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlWrite);
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
	}
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::ReadQuery_1997(VisaDriverIoctrlBasePtr ptr)
{

	int ret = 0;
	static int channel = -1;
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	DeviceDriverReadQuery* upper_arg = dynamic_cast<DeviceDriverReadQuery*>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	
	{
		//voltage
		std::string command = "";
		if (upper_arg->channel == -1 && channel != -1) {
			//backuped current channel
			upper_arg->channel = channel;
		}
		if (upper_arg->channel != -1) {
			//hand channel
			command.append(QString("INSTrument:NSELect %1;").arg(upper_arg->channel).toStdString());
			channel = upper_arg->channel;
		}
		command.append("MEASure:VOLTage?");
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			upper_arg->result = mptr->result;
			upper_arg->voltage_mv = QString(mptr->result.c_str()).toDouble()*1000;
		}
	}
	{
		//:CURRent
		std::string command = "";
		if (upper_arg->channel != -1) {
			//hand channel
			command.append(QString("INSTrument:NSELect %1;").arg(upper_arg->channel).toStdString());
		}
		command.append("MEASure:CURRent?");
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = command;

		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond.append(" ; ");
		ptr->commond.append(mptr->commond);
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			upper_arg->result.append(", ");
			upper_arg->result.append(mptr->result);
			upper_arg->current_ma = QString(mptr->result.c_str()).toDouble() * 1000;
		}
	}
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::ReadQuery_1999(VisaDriverIoctrlBasePtr ptr)
{
	std::string command=":READ";
	int ret = 0;
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	DeviceDriverReadQuery* upper_arg = dynamic_cast<DeviceDriverReadQuery*>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}

	command.append("?");
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			upper_arg->result = mptr->result;
			auto result = QString(mptr->result.c_str()).split(',');
			if (result.size() == 3) {
				foreach(auto res, result) {
					QString tmp = res.left(res.size() - 1);
					double value = tmp.toDouble();
					if(GlobalConfig_debugdevcieBase)qDebug("value %f res [%s]", value, tmp.toStdString().c_str());
					if (res.at(res.size() - 1).toUpper() == 'A') {
						upper_arg->current_ma = value*1000;
					}else
					if (res.at(res.size() - 1).toUpper() == 'V') {
						upper_arg->voltage_mv = value * 1000;
					}
					else
					if (res.at(res.size() - 1).toUpper() == 'S') {
						upper_arg->ativetime_ms = value * 1000;
					}
				}
			}
			else {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}


		}
	}
ERROR_OUT:
	return ret;
}
QString DeviceBase::GetDeviceSCPIVersion()
{
	QString result="";
	if (scpi_version.size() > 0) {
		result = scpi_version;
		goto ERROR_OUT;
	}
	if (mcommuinterface != DriverClass::DriverSCPI) {
		if (GlobalConfig_debugdevcieBase)qDebug("device %d not support",moffset_inlist);
		goto ERROR_OUT;
	}
	int ret = 0;
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		mptr->commond = "SYST:VERS?";
		VISA_DEVICE_IOCTRL(mptr)
		if (ret == 0) {
			if (mptr->result.size() <= 0) {
				ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
				goto ERROR_OUT;
			}
			result = QString(mptr->result.c_str());
			scpi_version = result;
			qDebug("device %d SCPI %s",moffset_inlist, mptr->result.c_str());
		}
	}
ERROR_OUT:
	return result;
}

int32_t DeviceBase::RelayChannel(VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	if (ptr == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (mcommuinterface != DriverClass::DriverRelayBMW) {
		if (GlobalConfig_debugdevcieBase)qDebug("device %d not support", moffset_inlist);
		goto ERROR_OUT;
	}
	DeviceDriverRelayChannelRW* upper_arg = dynamic_cast<DeviceDriverRelayChannelRW*>(ptr.get());
	if (upper_arg == nullptr) {
		qCritical("invaild upper_arg");
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	{
	//采用长格式命令
	VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
	std::string command = ""; 
	int max_channel = QString(GetIdentify().c_str()).right(2).toInt();
	if (upper_arg->is_read) {
		RelayBMWDriver::ReadChannelMaskString(command, max_channel);
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
			if (ret == 0) {
				//string (长格式)-->int(短格式)
				uint8_t cmdbuffer[64];
				int result_cn = RelayBMWDriver::StringToLocal8bit(mptr->result, cmdbuffer, sizeof(cmdbuffer));
				if (result_cn!= (max_channel+3) || cmdbuffer[0]!=0xfd || cmdbuffer[1] != 0x22 || cmdbuffer[max_channel + 3-1] != 0xdf) {
					qCritical("result unmatch");
					Utility::DumpHex(cmdbuffer, result_cn,"read result");
					ret = -ERROR_INVALID_PARAMETER;
					goto ERROR_OUT;
				}
				RelayBMWDriver::LongFormatStringToShortInt(cmdbuffer,sizeof(cmdbuffer), upper_arg->channelvalue);
				goto ERROR_OUT;
			}
	}
	else {
		//写
		command = "";
		RelayBMWDriver::WriteChannelMaskString(command, upper_arg->channelmask, upper_arg->channelvalue,max_channel);
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
			if (ret == 0) {
				//返回的是板的真实情况
				uint8_t cmdbuffer[64];
				int result_cn = RelayBMWDriver::StringToLocal8bit(mptr->result, cmdbuffer, sizeof(cmdbuffer));
				if (result_cn != (max_channel + 3) || cmdbuffer[0] != 0xfd || cmdbuffer[1] != 0x21 || cmdbuffer[max_channel + 3 - 1] != 0xdf) {
					qCritical("result unmatch");
					Utility::DumpHex(cmdbuffer, result_cn, "read result");
					ret = -ERROR_INVALID_PARAMETER;
					goto ERROR_OUT;
				}
			}
	}
	}
ERROR_OUT:
	return ret;
}