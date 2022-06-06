#include "TestCaseBGServer.h"
#include <winerror.h>
#include "TestCase.h"

TestCaseBGServerPtr TestCaseBGServer::_instance = NULL;

TestCaseBGServerPtr TestCaseBGServer::get_instance()
{
	if (!_instance)
	{
		TestCaseBGServerPtr ptr(new TestCaseBGServer);
		_instance = ptr;
	}
	return _instance;
}
TestStep TestCaseBGServer::getTestCaseStep()
{
	QMutexLocker locker(&mStepmutex);
	return this->mRunStep;
}

int32_t TestCaseBGServer::StepChangeRuningorPause()
{
	int ret = 0;
	switch (mRunStep) {
	case TestStep::Unknown:
	case TestStep::Starting:
		ret = ToRuning();
		break;
	case TestStep::Runing:
		ret = ToPauseing();
		break;
	case TestStep::Pause:
		ret = ToContinue();
		break;
	case TestStep::Termining:
	case TestStep::UserTermin:
	case TestStep::Complete:
		ret = ToRestart();
	default:
		break;
	}
	return ret;
}

int32_t TestCaseBGServer::SetNextStep(TestStep mstep, bool immediately)
{
	int ret = 0;
	if (TestStep::UserTermin == mstep) {
		SetStep(mstep);
	}
	return ret;
}

bool TestCaseBGServer::iscaseRuning()
{
	QMutexLocker locker(&mStepmutex);
	int ret = false;
	if (mRunStep <= TestStep::Continue) {//都属于正在进行
		ret = true;
	}
	return ret;
}
bool TestCaseBGServer::iscasePause()
{
	QMutexLocker locker(&mStepmutex);
	int ret = false;
	if (mRunStep == TestStep::Pause || TestStep::Pauseing== mRunStep) {//都属于正在进行
		ret = true;
	}
	return ret;
}
void TestCaseBGServer::SetStep(TestStep v) 
{
	QMutexLocker locker(&mStepmutex);
	if (GlobalConfig_debugtestcasebgserver) {
		qDebug("form %s to %s", StepToString(mRunStep.load()).c_str(), StepToString(v).c_str());
	}
	mRunStep = v;
}
void TestCaseBGServer::rebasestatus()
{
	mBGstatus.casesector = 0;
	mBGstatus.caseitemseek = 0;
}
int32_t TestCaseBGServer::HandleNextCaseitem()
{
	int32_t ret = 0;
	ret = TestcaseBase::get_instance()->HandleTestcase(mBGstatus.casesector, mBGstatus.caseitemseek);
	switch(ret) {
		case error_itemseekend:
			mBGstatus.casesector++;
			mBGstatus.caseitemseek = 0;
			ret = 0;
			break;
		case error_casesectorend:
			//完成一轮测试
		{
			auto msg = new MessageTVBGStatus;
			msg->issuccess = true;
			MessageTVBasePtr mptr(msg);
			emit notifytoView(int(msg->GetCmd()), mptr);
		}
			break;
		case ERROR_SUCCESS:
#if 0
			//notice view 由于需要更新文字数据，从casebase notice
		{
			auto msg = new MessageTVBGStatus;
			msg->issuccess = true;
			MessageTVBasePtr mptr(msg);
			emit notifytoView(int(msg->cmd), mptr);
		}
#endif
			mBGstatus.caseitemseek++;
			break;
		case error_caseusrtermin:
		default:
			qCritical("ret 0x%x",ret);
			{
				auto msg = new MessageTVBGStatus;
				msg->issuccess = false;
				msg->isusertermin = (ret == error_caseusrtermin);
				MessageTVBasePtr mptr(msg);
				emit notifytoView(int(msg->GetCmd()), mptr);
			}
			break;
	}
	return ret;
}
std::string TestCaseBGServer::StepToString(TestStep step)
{
	switch (step) {
	case TestStep::Starting: return std::string("Starting");
	case TestStep::Runing: return std::string("Runing");
	case TestStep::Pauseing: return std::string("Pauseing");
	case TestStep::Pause: return std::string("Pause");
	case TestStep::Continue: return std::string("Continue");
	case TestStep::UserTermin: return std::string("UserTermin");
	case TestStep::Complete: return std::string("Complete");
	case TestStep::Termining: return std::string("Termining");	
	case TestStep::Unknown: 
	default:
		return std::string("Unknown");
	}
}
int32_t TestCaseBGServer::ToStarting()
{
	int ret = 0;
	unsigned long timems = 10 * 1000;

	if (mautorunthread != nullptr) {
		//release old
		mautorunthread.reset();
	}
	
	{
		SetStep(TestStep::Starting);
		ThreadworkControllerPtr ptr(new ThreadworkController(std::bind(&TestCaseBGServer::mbgworkloop, this)));
		if (ptr == nullptr) { 
			ret = -ERROR_NOT_ENOUGH_MEMORY;
			goto ERR_OUT; 		
		}
		mautorunthread = ptr;
#if 1
		{
			QMutexLocker locker(&WORKTHREADMUTEX(mautorunthread));
			if (!(WORKTHREADWAIT(mautorunthread).wait(&WORKTHREADMUTEX(mautorunthread), timems))) {
				ret = -ERROR_NOT_ENOUGH_MEMORY;
				qDebug("mautorunthread setup fail");
			}
			else {
				qDebug("mautorunthread setup %p", mautorunthread);
			}
		}
#else
		_sleep(5000);
#endif
	}
	{
		rebasestatus();
		NotifyViewStep();
	}
ERR_OUT:
	return ret;
}
int32_t TestCaseBGServer::ToUserTermin()
{
	SetStep(TestStep::UserTermin);
	return 0;
}
int32_t TestCaseBGServer::ToComplete()
{
	SetStep(TestStep::Complete);
	return 0;
}
int32_t TestCaseBGServer::ToRestart()
{
	return ToRuning();
}
int32_t TestCaseBGServer::ToContinue()
{
	int ret = 0;
	if (ret == 0)SetStep(TestStep::Continue);
	return ret;
}
int32_t TestCaseBGServer::ToRuning()
{
	int ret = 0;
	if (mRunStep > TestStep::Continue)ret = ToStarting();
	if(ret==0)SetStep(TestStep::Runing);
	return ret;
}

int32_t TestCaseBGServer::ToTermin()
{
	SetStep(TestStep::Termining);
	return 0 ;
}

int32_t TestCaseBGServer::ToPause()
{
	int ret = 0;
	SetStep(TestStep::Pause);
	return ret;
}
int32_t TestCaseBGServer::ToPauseing()
{
	int ret = 0;
	SetStep(TestStep::Pauseing);
	return ret;
}
int32_t TestCaseBGServer::mbgworkloop()
{
	int ret = 0;
	int result = 0;
	if(GlobalConfig_debugtestcasebgserver)qDebug(" Enter");
	WORKTHREADWAIT(mautorunthread).notify_all();
	do {
		if (mRunStep==TestStep::Runing) {
			result = HandleNextCaseitem();
			if (result == 0)continue;
			if (result < 0) {
				ToTermin();
				break;
			}if (result == error_caseusrtermin) {
				ToUserTermin();
				break;
			}if (error_casesectorend== result) {
				ToComplete();
				break;
			}
			else {
				qInfo("leave loop mRunStep %d", mRunStep.load());
				break;
			}
		}
		else if (mRunStep==TestStep::Pauseing){
			ToPause();
			NotifyViewStep();
		}
		else if (TestStep::Continue==mRunStep) {
			{
			
			}
			ToRuning();
		}if (mRunStep <= TestStep::Pause) {
			_sleep(1000);
		}
		else {
			qInfo("leave loop mRunStep %d result %d", mRunStep.load(), result);
			break;
		}
	} while (mautorunthread->isthreadkeeprun());
	if (GlobalConfig_debugtestcasebgserver)qDebug(" Leaver step %d ret 0x%x result 0x%x", mRunStep.load(),ret, result);
	return ret;
}

void TestCaseBGServer::NotifyViewStep()
{
	auto msg = new MessageTVBGUpdate;
	msg->mstep = getTestCaseStep();
	MessageTVBasePtr mptr(msg);
	emit notifytoView(int(msg->GetCmd()), mptr);
}
