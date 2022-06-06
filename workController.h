#pragma once
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker> 
#include <QWaitCondition>
#include "WorkThread.h"
#define WORKTHREADWAIT(x) x->workthreadwait
#define WORKTHREADMUTEX(x) (x->workthreadmutex)
typedef std::function<int32_t (void)>threadworkfunc;
class ThreadworkController;
typedef std::shared_ptr<ThreadworkController> ThreadworkControllerPtr;
class ThreadworkController:public QObject {
	Q_OBJECT
signals:
	void RunThread(int);
public slots:
	void handleResults(int result);
public:
	ThreadworkController() = delete;
	ThreadworkController(threadworkfunc cb);
	~ThreadworkController();
	threadworkfunc getCallback() {
		return mcb;
	};
	bool isthreadkeeprun() {
		return threadkeeprun;
	};
	bool isthreadruning() {
		return mworkThread->isRunning();
	}
private:
	int32_t setupworkthread();

private:
	bool threadkeeprun = true;
	QThread* mworkThread=nullptr;
	threadworkfunc mcb;
public:
	QMutex workthreadmutex;

	QWaitCondition workthreadwait;
};
