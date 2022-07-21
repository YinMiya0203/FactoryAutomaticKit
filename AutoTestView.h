#pragma once

#include <QWidget>
#include "ui_AutoTestView.h"
#include <QGridLayout> 
#include <QDesktopWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QTextBrowser>
#include <QHeaderView>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QSpinBox>
#include "AutoTestCaseitemView.h"
#include "TestCase.h"
#include "QCountDownDialog.h"
#include <QInputDialog>
#include <QImage>
#include <QPainter>
#include "QSPMessageBox.h"
#if 0
typedef struct BoardSN_t {
	QString prex;
	int32_t sn;
}BoardSN;
#endif
typedef struct TestViewSettings_t {
	bool is_cycle=false;
	int32_t cycleintervalsecond = 10;
	int32_t success_cnt = 0;
	int32_t fail_cnt = 0;
	int32_t usertermin_cnt = 0;
	//BoardSN_t boardsn;
}TestViewSettings;

class AutoTestView :public QWidget
{
	Q_OBJECT
public:
	AutoTestView(QWidget* main_widget);
	AutoTestView()=delete;
	AutoTestView& operator=(const AutoTestView ) = delete;
	static QString GetVerionInfo();
	~AutoTestView();
	bool eventFilter(QObject* watched, QEvent* event) override;
private slots:
	void on_deviceconectpb_clicked();
	void on_devicetestactivepb_clicked();
	void on_test_start_pb_clicked();
	void on_test_termin_pb_clicked();
public slots:
	void messagefromtestcase(int, MessageTVBasePtr);
signals:
	void messagetodevice(int ,MessageFVBasePtr);
private:
	int32_t setuptitleView(QWidget* parent);
	int32_t setupdeviceView(QWidget* parent);
	int32_t setdeviceupiteminterface(QWidget* dev_widget, DeviceInfo_t dev,int32_t cntoffset, int32_t totalcolum);
	int32_t setdeviceupitem_auto(QWidget* dev_widget, DeviceInfo_t dev, int32_t cntoffset, int32_t totalcolum);
	int32_t setupsettingView(QWidget* parent);
	int32_t setuplogView(QWidget* parent);
	int32_t setuptestcaseView(QWidget* parent);

	int32_t DeleteView(QWidget* dev_widget);
	void RegisterSignalTotestcase();
	const int getMainViewGridRow() { return 22; };
	const int getMainViewGridColum() { return 3*4; };

	void DeviceWidgetFresh(int32_t index, MessageTVDeviceUpdate *msg = nullptr);
	void DeviceOutputStatusWidgetFresh(int32_t index, MessageTVDeviceUpdate* msg=nullptr);
	void DeviceConnectWdigetFresh(int32_t index, QPushButton* pb = nullptr);
	void DeviceActiveWdigetFresh(int32_t index, QPushButton* pb = nullptr);
	bool TestCastStartPauseWidgetFresh(QPushButton* pb, bool);
	bool TestCastStartTerminWidgetFresh(QPushButton* pb, bool);
	bool TestCastcycleStatusWidgetFresh(QLabel *label=nullptr);
	bool TestCastcycleStatusCleanWidgetFresh(QPushButton* pb = nullptr);
	bool TestCaseTableWidgetFresh(QTableWidget *pb=nullptr);

	void TestCaseBGWidgetFresh(MessageTVBGUpdate *msg = nullptr);

	void AutoTestSettingWidgetFresh();
	void HandleTestCaseOneShot(MessageTVBGStatus *msg=nullptr);
	void HandleCountDonwDialog(MessageTVCaseCountDownDialog* msg=nullptr);
	void HandleCaseNoticeDialog(MessageTVCaseNoticeDialog* msg = nullptr);
	void HandleCaseConfirmDialog(MessageTVCaseConfirmDialog* msg = nullptr);
	void HandleCaseConfirmWithinputDialog(MessageTVCaseConfirmWithInputsDialog* msg = nullptr);
	void HandleCaseItemWidgetStatus(MessageTVCaseItemWidgetStatus*msg = nullptr);
	void HandleCaseItemWidgetStringUpdate(MessageTVCaseItemWidgetStringUpdate* msg = nullptr);
	void HandleDeviceScanWidgetUpdate(MessageTVHardWareDeviceRes* msg = nullptr);
	void HandleLogWidgetUpdate(MessageTVLogWidgetUpdate* msg = nullptr);
	void HandleBGDiskSpace();


	void HandleTestCaseResultSave(int sector, int seek, QString value, QColor c);
	void HandleTestCaseResultSave();
	QString GetTestingBoardSN();
	int32_t TestingBoardSNWidgetIncrease(int increase = 1);
	void TestingBoardSNWidgetFresh();
	void CycleTestHandle();
	TestCaseitemcontainer mtcitemcontainer;
	QWidget* mparent_widget=nullptr;
	TestViewSettings_t msettings;
};
