#pragma once
#include <QMutex>
#include <QMutexLocker> 
#include <QWaitCondition>
#include <QMessageBox>
#include "VisaDriver.h"
#if 1
enum class MessageFromView {
	Unknown,
	FindDeviceRes,
	ConnectDisDevice,
	TestactiveDevice,
	TestCasePauseRun,
	TestCaseTermin,
};
#endif

enum class MessageToView {
	Unknown,
#if 0
	//µ¥ÏîË¢°É
	DeviceStatusBegin,
	DeviceStatusConnectUpdate,
	DeviceStatusONOFFUpdate,
	DeviceStatusVoltageUpdate,
	DeviceStatusCurrentUpdate,
	DeviceStatusTail,
#endif
	HardWareDeviceRes,
	DeviceStatusUpdate,
	TestCaseBGStatusUpdate,
	TestCaseBGOneShot,
	TestCaseHandleCountdownDialog,
	TestCaseHandleManualNoticeDialog,
	TestCaseHandleManualConfirmDialog,
	TestCaseHandleManualConfirmWithInputsDialog,
	TestCaseItemStatus,
	TestCaseItemStringMsgUpdate,
	BackGroundServiceMsgToLogWidget,
};
enum class DeviceStatusIcon {
	Unknown,
	Connect,
	OnOff,
	Voltage,
	Current,
	TestActive,
};
enum class TestStep {
	Starting,
	Runing,
	Pauseing,
	Pause,
	Continue,
	UserTermin,
	Complete,
	Termining,
	Unknown,
};
enum class TestCaseDialog {
	CountDown,

};
#if 1
class MessageTVBase;
typedef std::shared_ptr<MessageTVBase> MessageTVBasePtr;
class MessageTVBase {
public:
	MessageToView cmd = MessageToView::Unknown ;
	int32_t index = -1;
	virtual MessageToView GetCmd() { return cmd; };
	virtual std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		return raw;
	};
};
class MessageTVHardWareDeviceRes : public MessageTVBase {
private:
	MessageToView cmd = MessageToView::HardWareDeviceRes;
public:
	MessageToView GetCmd() { return cmd; };
	Resourcecontainer res;
	bool is_success=false;
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Res:"); raw.append(std::to_string(res.size()));
		raw.append(";OK:"); raw.append(std::to_string(is_success));
		for each (auto resstr in res)
		{
			raw.append("; "); raw.append(resstr);
		}
		return raw;
	};
};
class MessageTVDeviceUpdate :public MessageTVBase
{
private:
	MessageToView cmd = MessageToView::DeviceStatusUpdate ;
public:
	DeviceStatusIcon icon = DeviceStatusIcon::Unknown;
	MessageToView GetCmd() { return cmd; };
	virtual std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";Icon:"); raw.append(std::to_string(int(icon)));
		return raw;
	};
};
class MessageTVBGUpdate :public MessageTVBase
{
private:
	MessageToView cmd = MessageToView::TestCaseBGStatusUpdate;
public:
	TestStep mstep = TestStep::Unknown;
	MessageToView GetCmd() { return cmd; };
	virtual std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";step:"); raw.append(std::to_string(int(mstep)));
		return raw;
	};
};
class MessageTVBGStatus :public MessageTVBase
{
private:
	MessageToView cmd = MessageToView::TestCaseBGOneShot;
public:
	bool issuccess =false;
	bool isusertermin = false;
	MessageToView GetCmd() { return cmd; };
	virtual std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";Status:"); raw.append(std::to_string(int(issuccess)));
		raw.append(";termin:"); raw.append(std::to_string(int(isusertermin)));
		return raw;
	};

};

class MessageTVCaseCountDownDialog :public MessageTVBase
{
private:
	MessageToView cmd = MessageToView::TestCaseHandleCountdownDialog;
public:
	int32_t durationms;
	QString msg;
	QList<QString> resource;
	QMutex mutex;
	bool is_success = false;
	QWaitCondition mwait;
	MessageToView GetCmd() { return cmd; };
	virtual std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";DUR:"); raw.append(std::to_string(int(durationms)));
		return raw;
	};
};
class MessageTVCaseNoticeDialog :public MessageTVBase {
private:
	MessageToView cmd = MessageToView::TestCaseHandleManualNoticeDialog;
public:
	QString msg;
	//QStringList resource;
	QMutex mutex;
	QWaitCondition mwait;
	MessageToView GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";Msg:"); raw.append(msg.toStdString());
		return raw;
	};

};
class MessageTVCaseConfirmDialog :public MessageTVBase {
private:
	MessageToView cmd = MessageToView::TestCaseHandleManualConfirmDialog;
public:
	QString msg;
	QList<QString> resource;
	int32_t durationms=0;
	int32_t buttonclicked;
	QMutex mutex;
	QWaitCondition mwait;
	MessageToView GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";Msg:"); raw.append(msg.toStdString());
		raw.append(";Clicked:"); raw.append(std::to_string(buttonclicked));
		raw.append(";Duration:"); raw.append(std::to_string(durationms));
		return raw;
	};
};
class MessageTVCaseConfirmWithInputsDialog :public MessageTVBase {
private:
	MessageToView cmd = MessageToView::TestCaseHandleManualConfirmWithInputsDialog;
public:
	QStringList prex;
	QStringList input_value;
	//QStringList resource;
	QString msg;
	QMutex mutex;
	QWaitCondition mwait;
	MessageToView GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";Msg:"); raw.append(msg.toStdString());
		raw.append(";prex:"); raw.append(std::to_string(prex.size()));
		return raw;
	};
};
class MessageTVCaseItemWidgetStatus :public MessageTVBase {
private:
	MessageToView cmd = MessageToView::TestCaseItemStatus;
public:
	bool is_success=false;
	bool is_focus = false;
	int sector=0;
	int seek=0;
	MessageToView GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";sector:"); raw.append(std::to_string(sector));
		raw.append(";seek:"); raw.append(std::to_string(seek));
		raw.append(";succ:"); raw.append(std::to_string(is_success));
		raw.append(";focus:"); raw.append(std::to_string(is_focus));
		return raw;
	};
};
class MessageTVCaseItemWidgetStringUpdate :public MessageTVBase {
private:
	MessageToView cmd = MessageToView::TestCaseItemStringMsgUpdate;
public:
	QString msg;
	int sector = 0;
	int seek = 0;
	bool overwriteMode = false;
	MessageToView GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";sector:"); raw.append(std::to_string(sector));
		raw.append(";seek:"); raw.append(std::to_string(seek));
		raw.append(";msg:"); raw.append(msg.toStdString());
		raw.append(";overwriteMode:"); raw.append(std::to_string(overwriteMode));
		return raw;
	};
};
class MessageTVLogWidgetUpdate :public MessageTVBase {
private:
	MessageToView cmd = MessageToView::BackGroundServiceMsgToLogWidget;
public:
	QString msg;
	bool overwriteMode = false;
	MessageToView GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";msg:"); raw.append(msg.toStdString());
		raw.append(";overwriteMode:"); raw.append(std::to_string(overwriteMode));
		return raw;
	};
};
// FV
class MessageFVBase;
typedef std::shared_ptr<MessageFVBase> MessageFVBasePtr;
class MessageFVBase {
public:
	int32_t index = -1;
	MessageFromView cmd = MessageFromView::Unknown;
	virtual MessageFromView GetCmd() { return cmd; };
	virtual std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		return raw;
	};
};
class MessageFVDeviceConnectDis :public MessageFVBase
{
private:
	MessageFromView cmd = MessageFromView::ConnectDisDevice;
public:
	bool isconnect = false;
	std::string customerinterfaceid = {};
	MessageFromView GetCmd() { return cmd; };
	virtual std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		raw.append(";Connect:"); raw.append(std::to_string(isconnect));
		return raw;
	};
};
#if 1
class MessageFVFindDeviceRes :public MessageFVBase
{
private:
	MessageFromView cmd = MessageFromView::FindDeviceRes;
public:
	MessageFromView GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(";Index:"); raw.append(std::to_string(index));
		return raw;
	};
};
#endif

#endif
