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
	static int32_t NewFile(QString value);
	static int32_t NewDir(QString value);
	static const QString buildDateTime(QString prex="");
	static void OutputDebugPrintf(const char* format, ...);
	static QString GetWinErrorText(int32_t errcode);
};

