#pragma once
#include <iostream>
#include <thread>
#include <Windows.h>
class CWinLock
{
public:
    CWinLock(CRITICAL_SECTION* winsec) : m_winsec(winsec)
    {
        EnterCriticalSection(m_winsec); // 进入临界区
    }

    ~CWinLock()
    {
        LeaveCriticalSection(m_winsec);  // 离开临界区
    }

private:
    CRITICAL_SECTION* m_winsec = nullptr;
};

