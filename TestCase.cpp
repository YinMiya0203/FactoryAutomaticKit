#include "TestCase.h"
#include "GlobalSettings.h"

TestcaseBasePtr TestcaseBase::_instance=NULL;
static CRITICAL_SECTION g_winsec_settings;

TestcaseBasePtr TestcaseBase::get_instance()
{
	if (!_instance)
	{
		TestcaseBasePtr ptr(new TestcaseBase);
		_instance = ptr;
	}
	return _instance;
}

bool TestcaseBase::LoadFile(QString filename)
{
	bool ret = false;
	QSettings* settings;
	settings = new QSettings(filename, QSettings::IniFormat);

	if (settings == nullptr) {
		goto error;
	}
	settings->setIniCodec("UTF-8");
	if (on_run(settings) != 0) {
		ret = false;
		goto error;
	}
	ret = true;
error:
	delete settings;
	if (ret == true)runingcasename = filename;
	return ret;
}


TestcaseBase::TestcaseBase()
{
	mdevice_list.clear();
	mcase_list.clear();
	mmfvcontainer.clear();
	mmtvcontainer.clear();
}

TestcaseBase::settingsgroup_class TestcaseBase::getGroupclass(QString group)
{
	TestcaseBase::settingsgroup_class ret = settingsgroup_class::group_class_unknow;
	if (group.contains("Device", Qt::CaseInsensitive)) {
		ret = settingsgroup_class::group_class_device;
		goto ROUT;
	}
	if (group.contains("ini_version", Qt::CaseInsensitive)) {
		ret = settingsgroup_class::group_class_iniversion;
		goto ROUT;
	}
	if (group.contains("testcase", Qt::CaseInsensitive)) {
		ret = settingsgroup_class::group_class_testcaseitem;
		goto ROUT;
	}
	if (group.contains("commontag", Qt::CaseInsensitive)) {
		ret = settingsgroup_class::group_class_commontag;
	
	}
ROUT:
	return ret;
}
int32_t TestcaseBase::on_run(QSettings* settings)
{
	int ret = 0;
	bool device_inisuccess = true;
	bool testcase_inisuccess = true;
	minicommontagcontainer.clear();
	do {
		//CWinLock win_lock(&g_winsec_settings);//在报错
		if (settings == NULL) {
			ret = ERROR_INVALID_PARAMETER;
			break;
		}
		QStringList childs = settings->childKeys();
		foreach(auto key, childs) {
			auto value = settings->value(key).toString().toStdString();
			TestcaseBase::settingsgroup_class type = getGroupclass(key);
			if (GlobalConfig_debugtestcaseini)qDebug("childs key %s == %s",key.toStdString().c_str(), value.c_str());
			if (type == settingsgroup_class::group_class_iniversion) {
				ini_version = value;
			}
			else if (type==settingsgroup_class::group_class_commontag) {
				minicommontagcontainer.insert(key, settings->value(key).toString());
			}
		}
		//分级处理

		QStringList groups = settings->childGroups();
		foreach(auto group, groups) {
			if (GlobalConfig_debugtestcaseini)qDebug("group %s", group.toStdString().c_str());
			if (getGroupclass(group) == settingsgroup_class::group_class_device) {
				settings->beginGroup(group);
				DeviceBasePtr ptr = DeviceBase::get_instance(settings, mdevice_list.size());
				if (ptr != nullptr) {
					mdevice_list.push_back(ptr);
				}
				else {
					device_inisuccess = false;
					break;
				}
				settings->endGroup();
			}
			else if (getGroupclass(group) == settingsgroup_class::group_class_testcaseitem) {
				settings->beginGroup(group);
				CaseItemBasePtr ptr = CaseItemBase::get_instance(settings, mcase_list.size());
				if (ptr != nullptr) {
					mcase_list.push_back(ptr);
				}
				else {
					testcase_inisuccess = false;
					break;
				}
				settings->endGroup();
			}
			
		}
		if (!device_inisuccess) {
			mdevice_list.clear();
			ret = -ERROR_INVALID_PARAMETER;
		}
		if (!testcase_inisuccess) {
			mcase_list.clear();
			ret = -ERROR_INVALID_PARAMETER;
		}	
	} while (0);
	if (ret == 0) {
		RegisterSignals();
	}
	return ret;
}
void TestcaseBase::RegisterSignals()
{
	//qDebug(" ");
	foreach(auto dev, mdevice_list)
	{
		qRegisterMetaType<MessageTVBasePtr>("MessageTVBasePtr");
		connect(dev.get(), SIGNAL(notifytoView(int, MessageTVBasePtr)), this, SLOT(messagefromdevice(int, MessageTVBasePtr)));
	}
	connect(TestCaseBGServer::get_instance().get(), SIGNAL(notifytoView(int, MessageTVBasePtr)), this, SLOT(messagefromBG(int, MessageTVBasePtr)));
	foreach(auto caseitem, mcase_list)
	{
		connect(caseitem.get(), SIGNAL(notifytoView(int, MessageTVBasePtr)), this, SLOT(messagefromcaseitem(int, MessageTVBasePtr)));
	}
	setupworkthread();
}
void TestcaseBase::setupworkthread()
{
	if (msgthread == nullptr) {
		ThreadworkControllerPtr ptr(new ThreadworkController(std::bind(&TestcaseBase::msgthreadloop, this)));
		msgthread = ptr;
	}
}

QString TestcaseBase::get_runcasename()
{
	return runingcasename;
}
//device
int32_t TestcaseBase::GetDeviceId(std::string networkid, int32_t& id)
{
	int ret = -ERROR_DEVICE_UNREACHABLE;
	int mcounterunit = 0;
	if (networkid.size() <= 0)goto ERR_OUT;
	foreach(auto item, mdevice_list) {
		auto label = QString(item->GetNetworklabel().c_str()).toUpper();
		if (label.split('/').contains(QString(networkid.c_str()).toUpper())) {
			id = mcounterunit;
			ret = 0;
			break;
		}

		mcounterunit++;
	}
ERR_OUT:
	return ret;
}
bool TestcaseBase::InterfaceidSetable(int32_t offset)
{
	bool ret = false;
	bool alloffset = false;
	int foreach_offset = 0;
	if ((mdevice_list.size() == 0) || (offset >= (int32_t)(mdevice_list.size())))
	{
		qWarning("offset %d outof rang %d", offset, mdevice_list.size());
		goto ERR_OUT;
	}
	//qDebug("offset %d", offset);
	foreach(auto item, mdevice_list) {
		if (offset == -1) {
			alloffset = mdevice_list[foreach_offset]->InterfaceidSetable();
			if (alloffset == true) {
				ret = true;
				break;
			}
		}
		else if (foreach_offset==offset) {
			ret = mdevice_list[foreach_offset]->InterfaceidSetable();
			break;
		}
		foreach_offset++;
	}
ERR_OUT:

	return ret;
}
int32_t TestcaseBase::Get_deviceInfo(DeviceInfoList& info, int32_t offset)
{
	int ret = 0;
	int foreach_offset = 0;
	info.clear();

	if ( (mdevice_list.size()==0) || (offset >= (int32_t)(mdevice_list.size())) )
	{
		ret = -ERROR_INVALID_DATA;
		qWarning("offset %d outof rang %d",offset, mdevice_list.size());
		goto ERR_OUT;
	}
	foreach(auto item, mdevice_list) {
		if ((offset == -1) || (foreach_offset == offset)) {
			info.push_back(item->GetDeviceInfo());
		}
		foreach_offset++;
	}

ERR_OUT:

	return ret;
}
int32_t TestcaseBase::GettestcaseStepcnt()
{
	int cnt =  mcase_list.size();

	if (GlobalConfig_debugtestcaseverbose)qDebug("case cnt %d",cnt);
	return cnt;
}
int32_t TestcaseBase::Gettestcasetconditionitemcnt(int32_t offset) 
{
	if (offset >= mcase_list.size() )return 0;
	int ret =  mcase_list[offset]->GetConditioncnt();
	if(GlobalConfig_debugtestcaseverbose)qDebug("TestcaseSector[%d] conditioncnt %d", offset,ret);
	return ret;
}

int32_t TestcaseBase::Gettestcasetresultitemcnt(int32_t offset) 
{
	if (offset >= mcase_list.size() )return 0;
	int ret = mcase_list[offset]->GetResultitemcnt();
	if(GlobalConfig_debugtestcaseverbose)qDebug("offset %d result %d", offset, ret);
	return ret;
}
int32_t TestcaseBase::Gettestcaseitemstotal(int32_t offset)
{
	if (offset >= mcase_list.size() )return 0;
	int ret = mcase_list[offset]->Getitemstotal();
	return ret;

}
int32_t TestcaseBase::Gettestcaseitemshow(int offset_testcase,int offset_item, ShowString &output)
{
	int ret = 0;
	if (offset_testcase >= mcase_list.size() ) {
		return -ERROR_INVALID_PARAMETER;
	}
	ret = mcase_list[offset_testcase]->Getitemshow(offset_item, output);
	return ret;
}
int32_t TestcaseBase::Gettestcasetypeshow(int offset_testcase, caseitem_class type, ShowString & output)
{
	int ret = 0;
	if (GlobalConfig_debugtestcaseverbose)qDebug("offset %d type %d ",offset_testcase,type);
	if (offset_testcase >= mcase_list.size() ) {
		return -ERROR_INVALID_PARAMETER;
	}
	ret = mcase_list[offset_testcase]->Getitemtypeshow(type, output);
	return ret;
}
QString TestcaseBase::GettestcasetConditionShowTotal(int32_t offset_testcase)
{
	int ret = 0;
	if (GlobalConfig_debugtestcaseverbose)qDebug("offset %d ", offset_testcase);
	if (offset_testcase >= mcase_list.size()) {
		return "";
	}
	return mcase_list[offset_testcase]->GetConditionShowTotal();
}
caseitem_type TestcaseBase::Gettestcaseitemtype(int offset_testcase, int offset_item) 
{
	if (offset_testcase >= mcase_list.size() ) return caseitem_type::unknown;
	return mcase_list[offset_testcase]->Getitemtype(offset_item);
}
void TestcaseBase::SetActiveCaseSector(QList<int> v)
{
	if (GLOBALSETTINGSINSTANCE->isUserRoot()) {
	active_casesector = v;
	}
}
int32_t TestcaseBase::devicetestactive(int32_t index)
{
	int ret = 0;
	if (index >= mdevice_list.size()) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERR_OUT;
	}

	ret = mdevice_list[index]->testactiveasync();

ERR_OUT:
	return ret;
}

int32_t TestcaseBase::devcieioctrl(int32_t index,VisaDriverIoctrlBasePtr ptr)
{
	int ret = 0;
	VisaDriverIoctrl cmd = ptr->GetCmd();
	if (index >= mdevice_list.size()) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERR_OUT;
	}

	ret = mdevice_list[index]->ioctrl(ptr);

ERR_OUT:
	if (ret != 0) {
		qCritical("ret %d index %d cmd %d", ret,index,cmd);
	}
	return ret;
}
int32_t TestcaseBase::deviceconnectdis(int32_t index, bool isconnect, std::string customerinterfaceid)
{
	int ret = 0;
	if (index >=mdevice_list.size()) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERR_OUT;
	}
	//qDebug("index %d isconnect %d", index, isconnect);
	if (isconnect) {
		ret = mdevice_list[index]->connectasync(isconnect,customerinterfaceid);
	}
	else {
		ret = mdevice_list[index]->disconnectasync();
	}
ERR_OUT:
	return ret;
}
int32_t TestcaseBase::Getdevicestatus(int32_t index, DeviceStatus_t& st)
{
	int ret = 0;
	if (index >= mdevice_list.size()) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERR_OUT;
	}
	ret = mdevice_list[index]->GetdeviceStatus(st);
	if (ret == 0) {
		//起device线程
	}
ERR_OUT:
	return ret;

}
void TestcaseBase::messagefromview(int cmd, MessageFVBasePtr ptr)
{
	//auto devp = ptr.get();
	mmfvcontainer.push_back(ptr);
	WORKTHREADWAIT(msgthread).notify_all();
	//while (true) {};会卡住所有人啊,我又有见过异步的
}
void TestcaseBase::messagefromdevice(int cmd, MessageTVBasePtr ptr)
{
	//nead modify
	if (ptr->GetCmd() == MessageToView::HardWareDeviceRes) {
		DeviceInfoList info;
		int ret = Get_deviceInfo(info);
		MessageTVHardWareDeviceRes* msg = dynamic_cast<MessageTVHardWareDeviceRes*>(ptr.get());
		//qDebug("msg %s", msg->to_string().c_str());
		if(ret==0 && msg!=nullptr){
			foreach(auto iteminfo, info) {
				//qDebug("iteminfo %s", iteminfo.interfaceid.c_str());
				auto itemres = std::find(msg->res.begin(), msg->res.end(), iteminfo.interfaceid);
				if(itemres!= msg->res.end()){
						itemres->append(" [Connected]");
						qInfo("%s had connected", itemres->c_str());
				}				
			}
		}
		else {
			qCritical("HardWareDeviceRes ret %d",ret);
			goto ERROR_OUT;
		}
	}
	//qDebug("msg %s", ptr->to_string().c_str());
	emit messagetctoview(cmd, ptr);
ERROR_OUT:
	return;
}
void TestcaseBase::messagefromBG(int cmd, MessageTVBasePtr ptr)
{
	emit messagetctoview(cmd, ptr);
}
void TestcaseBase::messagefromcaseitem(int cmd, MessageTVBasePtr ptr)
{
	emit messagetctoview(cmd, ptr);
}
int32_t TestcaseBase::Handle_mfv() 
{
	int ret = 0;
	if (mmfvcontainer.size() == 0)return ret;
	auto payload = mmfvcontainer.begin();
	MessageFromView cmd = payload->get()->GetCmd();
	int index = payload->get()->index;
	//qDebug(" cmd %d ", cmd);
	switch (cmd) {
		case MessageFromView::ConnectDisDevice:
		{
			auto upper_arg = dynamic_cast<MessageFVDeviceConnectDis*>(payload->get());
			if (upper_arg != nullptr) {
				ret = deviceconnectdis(index, upper_arg->isconnect, upper_arg->customerinterfaceid);
			}
			else {
				ret = -ERROR_INVALID_DATATYPE;
			}
		}
			break;
		case MessageFromView::TestactiveDevice:
			ret = devicetestactive(index);
			break;
#if 1
		case MessageFromView::FindDeviceRes:
			{
				//auto upper_arg = dynamic_cast<MessageFVFindDeviceRes*>(payload->get());
				
				if (mdevice_list.size() <= index) {
					ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
					break;
				}
				mdevice_list[index]->FindResourcecontainer();
			}
			break;
#endif
		case MessageFromView::TestCasePauseRun:
		{
			//check deviceconnented
			ret = TestcasePauseStart();
		}
		break;
		case MessageFromView::TestCaseTermin:
		{
			ret = TestCaseBGServer::get_instance()->SetNextStep(TestStep::UserTermin);
		}
		break;
		default:
			break;
	};
ERR_OUT:
	mmfvcontainer.pop_front();
	return ret;
}
int32_t TestcaseBase::msgthreadloop()
{
	QMutexLocker locker(&WORKTHREADMUTEX(msgthread));
	
	int ret = 0;
	do {
		if(GlobalConfig_debugthread)qDebug(" enter wait");
		if (mmfvcontainer.empty() && mmtvcontainer.empty()) {
			WORKTHREADWAIT(msgthread).wait(&WORKTHREADMUTEX(msgthread));
		}
		if(GlobalConfig_debugthread)qDebug(" leave wait ");
#if 1
		while (msgthread->isthreadkeeprun() && !mmtvcontainer.empty()) {
			//qDebug("mmtvcontainer size %d ", mmtvcontainer.size());
		}
#endif
		while (msgthread->isthreadkeeprun() && !mmfvcontainer.empty()) {
			//qDebug("mmfvcontainer size %d ", mmfvcontainer.size());
			Handle_mfv();
		}
	} while (msgthread->isthreadkeeprun());

	return ret;
}
TestStep TestcaseBase::GetTestcaseBGStep()
{
	return TestCaseBGServer::get_instance()->getTestCaseStep();
}
bool TestcaseBase::isRuncase()
{
	return TestCaseBGServer::get_instance()->iscaseRuning();
}
bool TestcaseBase::isPausecase()
{
	return TestCaseBGServer::get_instance()->iscasePause();;
}
QString TestcaseBase::GetCommontag(QString key)
{
	return minicommontagcontainer[key];
}
int32_t TestcaseBase::TestcasePauseStart(void)
{
	int ret = 0;
	//check device connect
	{
		foreach(auto dev, mdevice_list) {
			DeviceStatus_t st;
			dev->GetdeviceStatus(st);
			if(st.connected==false)ret = dev->connectasync();
			if (ret != 0)goto ERR_OUT;
		}
	}

	ret = TestCaseBGServer::get_instance()->StepChangeRuningorPause();
	if (GlobalConfig_debugtestcasebgserver)qDebug("ret %d", ret);
ERR_OUT:
	return ret;
}
int32_t TestcaseBase::HandleTestcase(int sector, int seek)
{
	int ret = 0;
	do{
		
		if (sector > mcase_list.size()) {
			ret = -ERROR_INVALID_BLOCK;
			break;
		}
		if (sector==mcase_list.size()) {
			ret = error_casesectorend;
			break;
		}

		if (mcase_list[sector]->isCaseIgnore() || (active_casesector.size()>0 && (active_casesector.count(sector)==0)) ) {
			if (active_casesector.size() > 0) {
				qWarning("active_casesector size %d", active_casesector.size());
			}
			ret = error_itemseekend;
			break;
		}

		ret = mcase_list[sector]->Process(seek);
	} while (0);
	if (GlobalConfig_debugtestcasebgserver)qDebug("sector %d: seek %d result 0x%x", sector, seek, ret);
	return ret;
}
