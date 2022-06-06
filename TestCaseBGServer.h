#pragma once
#include <atomic>
#include <memory>
#include <QMutex>
#include <QObject>
#include "MessagePacket.h"
#include "workController.h"
#include <winerror.h>
#include "GlobalConfig.h"
class TestCaseBGServer;
typedef std::shared_ptr<TestCaseBGServer> TestCaseBGServerPtr;

#define   error_itemseekend ERROR_SEEK
#define   error_casesectorend ERROR_SECTOR_NOT_FOUND
#define   error_casenotoview ERROR_DEVICE_FEATURE_NOT_SUPPORTED
#define   error_caseignore ERROR_INVALID_TOKEN
#define   error_caseusrtermin ERROR_LOCK_VIOLATION

typedef struct TestCaseBGStatus_t
{
	int casesector = 0;
	int caseitemseek = 0;
	bool routed = false;
}TestCaseBGStatus;
//只管锁页面，运行进度移动，noticetoview
class TestCaseBGServer:public QObject
{
	Q_OBJECT
signals:
	void notifytoView(int, MessageTVBasePtr);
public:
	static TestCaseBGServerPtr get_instance();
	TestStep getTestCaseStep();
	int32_t StepChangeRuningorPause();
	int32_t SetNextStep(TestStep mstep= TestStep::Unknown, bool immediately=true);
	bool iscaseRuning();
	bool iscasePause();
	void destroy();
private:
	static TestCaseBGServerPtr _instance;
	int32_t ToRuning();
	int32_t ToTermin();
	int32_t ToPause();
	int32_t ToPauseing();
	int32_t ToStarting();
	int32_t ToUserTermin();
	int32_t ToComplete();
	int32_t ToRestart();
	int32_t ToContinue();
	void SetStep(TestStep v);
	int32_t mbgworkloop();
	void NotifyViewStep();
	void rebasestatus();
	int32_t HandleNextCaseitem();
	std::string StepToString(TestStep step);
	QMutex mStepmutex;
	std::atomic<TestStep>mRunStep = TestStep::Unknown;
	TestCaseBGStatus_t mBGstatus;
	ThreadworkControllerPtr  mautorunthread = nullptr;
};

