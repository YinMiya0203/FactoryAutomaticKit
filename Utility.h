#pragma once
#include <cstdint>
#include <QString>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QLocale>
#include <QTime>
#include <windows.h>
#include "GlobalConfig.h"
class Utility
{
public:
	static int32_t NewFile(QString value, bool ishidden=false);
	static int32_t NewDir(QString value);
	static const QString buildDateTime(QString prex="");
	static void OutputDebugPrintf(const char* format, ...);
	static QString GetWinErrorText(int32_t errcode);
	static qint64 GetFileSize_B(QString path);
	static qint64 GetFileSize_KB(QString path);
	static void Sleep(unsigned long _Duration);
};

