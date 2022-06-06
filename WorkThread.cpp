#include "WorkThread.h"

WorkThreadIMP::WorkThreadIMP(QObject* parent)
{
}
void WorkThreadIMP::run_loop(int p)
{
	loop_func(p);
}
