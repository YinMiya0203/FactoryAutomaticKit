#pragma once
#include <iostream>
#include <thread>
#include <Windows.h>
class CWinLock
{
public:
    CWinLock(CRITICAL_SECTION* winsec) : m_winsec(winsec)
    {
        EnterCriticalSection(m_winsec); // �����ٽ���
    }

    ~CWinLock()
    {
        LeaveCriticalSection(m_winsec);  // �뿪�ٽ���
    }

private:
    CRITICAL_SECTION* m_winsec = nullptr;
};

