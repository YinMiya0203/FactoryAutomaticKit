#pragma once
#include <qobject.h>
class WorkThreadIMP :
    public QObject
{
    Q_OBJECT
public:
    explicit WorkThreadIMP(QObject *parent = nullptr);
public slots:
    void run_loop(int p);
signals:
    void resultReady(const int result);
public:
    virtual void loop_func(int p) = 0;
};
template <class T>
class ObjectWorkThread :public WorkThreadIMP
{
public:
    void loop_func(int p) {
        T* send = qobject_cast<T*>(sender());
        auto func =  send->getCallback();
        int ret = (func==nullptr)?0:func();
        emit resultReady(ret);
    };

};
