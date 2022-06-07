#include "DeviceBase.h"
//#include "DeviceBase.h"
#include "visa.h"
#include <winerror.h>
#include <QRegularExpression>
DeviceBasePtrContainer DeviceBase::mstaticdeviceptrcontainer = {};

#define VISA_DEVICE_IOCTRL(xxx) \
		if (!isVirtualDevice())ret = interior_driver->ioctrl(xxx); \
		else { \
			qDebug("enter debug sleep"); \
			_sleep(1 * 1000); \
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
	if (mmap.find(QString(MAXPOWERWVA)) != mmap.end()) {
		maxva = mmap[QString(MAXPOWERWVA)];
	}
	DriverClass mdriverclass= DriverClass::DriverSCPI;
	if (mmap.find(QString(COMMUINTERFACE)) != mmap.end()) {
		QString comm = QString(mmap[QString(COMMUINTERFACE)].c_str());
		if (comm.toUpper()=="DMMI") {
			mdriverclass = DriverClass::DriverDMMIVictor;
		}
	}
	DeviceBasePtr ptr(new DeviceBase(offset,
		mmap[QString(IDENTIFY_STRING)],
		mmap[QString(NETWORKlabel_STRING)],
		mmap[QString(INTERFACEID_STRING)],
		mmap[QString(ASRLBDPSF_STRING)],
		maxva,
		mdriverclass));
	mstaticdeviceptrcontainer.push_back(ptr);
	return ptr;
}
bool strcasecmp(const char* s1, const char* s2) {
	return QString::compare(QString(s1), QString(s2), Qt::CaseInsensitive);

}
DeviceBase::DeviceBase(int offset,std::string iden, std::string net, std::string id, std::string confg,std::string maxva, DriverClass driverclass):
	moffset_inlist(offset),identifyorig(iden),networklabel(net),interfaceidorig(id), arslconfgstr(confg), mcommuinterface(driverclass)
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
	if (mcommuinterface == DriverClass::DriverSCPI) {
		interior_driver = NiDeviceDriverBasePtr(new VisaDriver);
		mdevice_class = DeviceClass::DeviceClass_DC;
	}
	else {
		interior_driver = NiDeviceDriverBasePtr(new VictorDMMIDriver);
		mdevice_class = DeviceClass::DeviceClass_digit_multimeter;
		interior_driver->SetCmdPostfix("\r\n");
	}
	interior_driver->SetIndexInList(moffset_inlist);
	mdevicestatus.output = false;
	mdevicestatus.connected = false;
	interfaceidcustomer.clear();
	FVcontainer.clear();
	if (maxva.size() > 0) {
		maxWVA = atoi(maxva.c_str());
	}
	mdevicestatus.maxWVA = maxWVA;
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
	if (mdevice_class !=DeviceClass::DeviceClass_digit_multimeter) {
		auto cmd = new DeviceDriverOutputState;
		VisaDriverIoctrlBasePtr ptr(cmd);
		return ioctrl(ptr);
	}
	else {
		auto cmd = new DeviceDriverReadQuery;
		VisaDriverIoctrlBasePtr ptr(cmd);
		auto type = GetNetworklabel().front();
		if (type == 'V') {
			cmd->mMeasfunc = DeviceDriverReadQuery::QueryMeasFunc::MeasDCV;
		}
		else if (type == 'I' || type == 'A') {
			cmd->mMeasfunc = DeviceDriverReadQuery::QueryMeasFunc::MeasDCI;
		}
		return ioctrl(ptr);
	}
	
}
int32_t DeviceBase::connectsync(std::string customerinterfaceid)
{
	if (GlobalConfig_debugdevciedriver)qDebug("index %d customerinterfaceid [%s]",moffset_inlist, customerinterfaceid.c_str());
	int ret = 0;
	{
		std::string tmp = GetInterfaceId();
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
			_sleep(1 * 1000);//for debug 
		}
		if (ret == 0) {
			SetInterfaceIdCustomer(tmp);
		}
	}
	if (ret == 0 && (arslconfgstr.size() > 0 || QString(GetInterfaceId().c_str()).toUpper().contains("ASRL")))
	{
		if (!isVirtualDevice())ret = interior_driver->Driversetattribute(masrlconfg);
	}
	if (ret ==0) {
		mdevicestatus.connected = true;
	}
	else {
		mdevicestatus.connected = false;
	}
	if (QString(GetIdentify().c_str()).toUpper() == "AUTO") {
		auto msg = new VisaDriverIoctrlBase;
		msg->cmd = VisaDriverIoctrl::ReadIdentification;
		auto ptr = VisaDriverIoctrlBasePtr(msg);
		Readidentification(ptr);
	}
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::disconnectasync()
{
	return connectasync(false);
}
int32_t DeviceBase::disconnectsync()
{
	if (GlobalConfig_debugdevciedriver)qDebug("index %d ", moffset_inlist);
	int ret = 0;
	if (!mdevicestatus.connected)return ret;
	if(mdevice_class == DeviceClass::DeviceClass_DC_BatterySimulator ||
		mdevice_class == DeviceClass::DeviceClass_DC) {
		//先关闭输出
		auto mptrsv = VisaDriverIoctrlBasePtr(new DeviceDriverSourceVoltage);
		DeviceDriverSourceVoltage* upper = dynamic_cast<DeviceDriverSourceVoltage*>(mptrsv.get());
		upper->is_read = false;
		upper->voltage_mv = 0;
		ret = SourceVoltageAmplitude(mptrsv);
	
	}
	if (!isVirtualDevice()) ret= interior_driver->Driverclose();
	if (ret == 0) {
		mdevicestatus.connected = false;
	}
	else {
		mdevicestatus.connected = true;
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
		ThreadworkControllerPtr ptr(new ThreadworkController(std::bind(&DeviceBase::threadloop, this)));
		msgthread = ptr;
	}
}

#if 1
int32_t DeviceBase::threadloop()
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
	default:
		qCritical("cmd %d unsupport",cmd);
		ret = -ERROR_INVALID_PARAMETER;
		break;
	}
ERROR_OUT:
	if (ret != 0) {
		if(GlobalConfig_debugdevcieBase)qDebug("ret %d cmd %d", ret,cmd);
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
	if (upper_arg->is_read) {
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
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
			std::string command = ":OUTPut";
			if (upper_arg->onoff) {
				command.append(" 1");
			}
			else {
				command.append(" 0");
			}
			mptr->commond = command;
			VISA_DEVICE_IOCTRL(mptr)
			ptr->commond = mptr->commond;
		}
	
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::SourceCurrentAmplitude(VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	std::string command;
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
	if (upper_arg->is_read) 
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		command = ":SOURce";
		if (upper_arg->channel >= 0) {
			command.append(std::to_string(upper_arg->channel));
		}
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
			upper_arg->current_ma = atoi(mptr->result.c_str()) * 1000;
		}
	}
	else {
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlWrite);
		command = ":SOURce";
		if (upper_arg->channel >= 0) {
			command.append(std::to_string(upper_arg->channel));
		}
		command.append(":CURRent ");
		float current_A = (float)upper_arg->current_ma / 1000;
		command.append(std::to_string(current_A));
		mptr->commond = command;
		VISA_DEVICE_IOCTRL(mptr)
		ptr->commond = mptr->commond;
		upper_arg->result = mptr->result;
		if (GetDeviceSCPIVersion() < SCPI_VERSION_1999) {
			if (GlobalConfig_debugdevcieBase)qDebug("force sleep");
			_sleep(2000);
		}
	}
ERROR_OUT:
	return ret;
}
int32_t DeviceBase::SourceVoltageAmplitude(VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	std::string command;
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
	if (upper_arg->is_read)
	{
		VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
		command = ":SOURce";
		if (upper_arg->channel >= 0) {
			command.append(std::to_string(upper_arg->channel));
		}
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
			upper_arg->voltage_mv = atoi(mptr->result.c_str()) * 1000;
		}
	}
	else {
			VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlWrite);
			command = ":SOURce";
			if (upper_arg->channel >= 0) {
				command.append(std::to_string(upper_arg->channel));
			}
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
			if (ret == VI_ERROR_TMO) {
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
				Sleep(3 * 1000);
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
int32_t DeviceBase::ReadQuery_1997(VisaDriverIoctrlBasePtr ptr)
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
		//voltage
		std::string command = "MEASure:VOLTage?";
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
			upper_arg->voltage_mv = QString(mptr->result.c_str()).toDouble();
		}
	}
	{
		//:CURRent
		std::string command = "MEASure:CURRent?";
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
			upper_arg->current_ma = QString(mptr->result.c_str()).toDouble();
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
	if (upper_arg->channel > 0) {
		command.append(std::to_string(upper_arg->channel));
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
	QString result;
	if (scpi_version.size() > 0) {
		result = scpi_version;
		goto ERROR_OUT;
	}
	int ret = 0;
	if (result.size() > 0)goto ERROR_OUT;
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
			if (GlobalConfig_debugdevcieBase)qDebug("device %d SCPI %s",moffset_inlist, mptr->result.c_str());
		}
	}
ERROR_OUT:
	return result;
}