#include "workController.h"

ThreadworkController::~ThreadworkController() {
	if(GlobalConfig_debugthread)qDebug("%p ",this);
	threadkeeprun = false;
	workthreadwait.notify_all();
	mworkThread->quit();
	mworkThread->wait();
	delete mworkThread;
}
void ThreadworkController::handleResults(int result)
{
	{
		qInfo("quit result %d \n",result);
		//_sleep(2000);
		if(result!=0)setupworkthread();
	}
}
#if 1
int32_t ThreadworkController::setupworkthread()
{
	int ret = 0;
	if(mworkThread==nullptr){
		mworkThread = new QThread;
		if (!mworkThread->isRunning()) {
			auto* worker = new ObjectWorkThread<ThreadworkController>;
			worker->moveToThread(mworkThread);
			connect(this, SIGNAL(RunThread(int)), worker, SLOT(run_loop(int)));
			//this->connect(this, SIGNAL(inherited::RunThread(int)), worker, SLOT(run_loop(int)));
			connect(mworkThread, &QThread::finished, worker, &QObject::deleteLater);
			connect(worker, SIGNAL(resultReady(int)), this, SLOT(handleResults(int)));
			mworkThread->start();
		}
		else {
			qInfo("mworkThread.isRunning() %d", mworkThread->isRunning());
		}
	}
	if (threadkeeprun)emit RunThread(0);
	return ret;
}
#endif
ThreadworkController::ThreadworkController(threadworkfunc cb)
{
	mcb = cb;
	setupworkthread();
}
