#pragma once
#include <cstdint>
#include <string>
#include <QString>
#include <winerror.h>
#include <QTableWidget>
#include <QFileDialog>
#include "GlobalSettings.h"
#include <memory>
#define SAVEASTABLE 
class ResultFileInfo {
	//自动命名
public:
	ResultFileInfo(QString prex);
	//ResultFileInfo();
	~ResultFileInfo();
	int32_t Fill(int sector, int seek, QString prex, QString value, QColor c);//一开始是个tmp文件，析构的时候rename一下
	int32_t Fill(QTableWidget* table, QString prex);
	bool isFull() { return mfilledcnt >= Maxfill; };
private:
	int32_t FormatTable(int sector, int seek, QString prex, QString value, QColor c, QString &output);
	int32_t FormatTable(QTableWidget* table, QString prex,QString &output);
	int32_t SetupNewfile();
	int32_t FillHead();
	int32_t FillEnd(QString filename);

	QFile mfile;
	int32_t mfilledcnt = 0;
	int32_t mfailedcnt = 0;
	QString mStartPrex = "NULL";
	QString mcurrentprex = "NULL";
	QDateTime mStarttime;
#ifdef SAVEASTABLE
	int32_t Maxfill = 50;
#else
	int32_t Maxfill = 500;
#endif
	QString mcurrentlogdir="";
	int32_t mCurrentSector = -1;
};
typedef std::shared_ptr<ResultFileInfo> ResultFileInfoPtr;

class TestCaseResultSaveICMP;
typedef std::shared_ptr<TestCaseResultSaveICMP> TestCaseResultSaveICMPPtr;

class TestCaseResultSaveICMP
{
public:
	static TestCaseResultSaveICMPPtr get_instance();
	int32_t Save(int sector, int seek, QString prex, QString value, QColor c);
	int32_t Save(QTableWidget* table, QString prex);
	int32_t SyncToDisk();//析构mresultfile;
	~TestCaseResultSaveICMP();
	bool isSaveAsTable() {
#ifdef SAVEASTABLE
		return true;
#else
		return false;
#endif
	};
private:
	explicit TestCaseResultSaveICMP();

	static TestCaseResultSaveICMPPtr __instance;
	ResultFileInfoPtr mresultfile=nullptr;
};

