#pragma once
#include "Utility.h"
#if 0
#define VISADRIVER_IO	0x0f
#define VISADRIVER_IOR	(0x01 << 0)
#define VISADRIVER_IOW	(0x01 << 1)
#define VISADRIVER_IOWR	(VISADRIVER_IOR | VISADRIVER_IOW)

#define VISA_CUST_IO(nr)	(nr<<8)
#define VISA_CUST_IO_MASK(nr)	(nr & (0x0ff<<8))
//#define VISA_CUST(nr)	(VISA_CUST_IO_MASK(nr)>>8)
#define CUSTOMER_SYSTEMERROR	0x01
#define CUSTOMER_READIDEN	0x02
#define CUSTOMER_SETOUTPUTSTATE	0x03
#define CUSTOMER_SETSOURCEVOLTAGE 0x04
#define CUSTOMER_SETSOURCECURRENT 0x05
#define VISA_IOCUST_SYSTEMERROR	(VISA_CUST_IO(CUSTOMER_SYSTEMERROR))
#define VISA_IOCUST_READIDEN	(VISA_CUST_IO(CUSTOMER_READIDEN))
#endif
enum class VisaDriverIoctrl
{
	Io = 0,
	IoRead,
	IoWrite,
	ReadSystemError,
	ReadSystemErrorCount,
	ReadIdentification,	//5
	OutputState,
	SourceVoltage,
	SourceCurrentLimit,
	ReadQuery,
	WorkFunction,
	RelayChannelRW,
	Io_max,
};
enum class DeviceWorkFunc {
	ENTRy,//enter startup screen
	POWer,//Power supply
	TEST,//Battery test
	SIMulator //Battery simulator
};
extern QStringList VisaDriverIoctrlList();
class VisaDriverIoctrlBase
{
public:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::Io;
	std::string commond;
	std::string result;
	virtual VisaDriverIoctrl GetCmd() { return cmd; };
	virtual ~VisaDriverIoctrlBase() {};
	virtual std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(result);
		return raw;
	};
};
typedef std::shared_ptr<VisaDriverIoctrlBase> VisaDriverIoctrlBasePtr;
//base
class VisaDriverIoctrlRead :public VisaDriverIoctrlBase
{
private:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::IoRead;
public:
	VisaDriverIoctrl GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(result);
		return raw;
	};
};
typedef std::shared_ptr<VisaDriverIoctrlRead> VisaDriverIoctrlReadPtr;

class VisaDriverIoctrlWrite :public VisaDriverIoctrlBase
{
private:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::IoWrite;
public:
	int32_t result_status;
	VisaDriverIoctrl GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(std::to_string(result_status));
		return raw;
	};
};
typedef std::shared_ptr<VisaDriverIoctrlWrite> VisaDriverIoctrlWritePtr;
//system
class DeviceDriverReadSystemErrorCount :public VisaDriverIoctrlBase
{
protected:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::ReadSystemErrorCount;
public:
	int32_t result_cnt = -1;
	VisaDriverIoctrl GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(result);
		raw.append(" result_Cnt:"); raw.append(std::to_string(result_cnt));
		return raw;
	};
};
typedef std::shared_ptr<DeviceDriverReadSystemErrorCount> DeviceDriverReadSystemErrorCountPtr;
class DeviceDriverWorkFunction :public VisaDriverIoctrlBase
{
protected:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::WorkFunction;
public:
	bool is_read = true;
	DeviceWorkFunc wfunctions = DeviceWorkFunc::ENTRy;
	VisaDriverIoctrl GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(result);
		raw.append(" func:"); raw.append(std::to_string(int(wfunctions)));
		raw.append(" is_read:"); raw.append(std::to_string(is_read));
		return raw;
	};
};
//physics
class DeviceDriverOutputState :public VisaDriverIoctrlBase
{
protected:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::OutputState;
public:
	VisaDriverIoctrl GetCmd() { return cmd; };
	bool is_read = true;
	bool onoff = false;
	int32_t channel = -1;
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(result);
		raw.append(" onoff:"); raw.append(std::to_string(onoff));
		raw.append(" is_read:"); raw.append(std::to_string(is_read));
		raw.append(" channel:"); raw.append(std::to_string(channel));
		return raw;
	};

};
typedef std::shared_ptr<DeviceDriverOutputState> DeviceDriverOutputStatePtr;
//SourceVoltage,
//SourceCurrent,
class DeviceDriverSourceVoltage :public VisaDriverIoctrlBase
{
protected:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::SourceVoltage;
public:
	bool is_read = true;
	int32_t voltage_mv = 0;
	int32_t channel = -1;
	VisaDriverIoctrl GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(result);
		raw.append(" voltage_mv:"); raw.append(std::to_string(voltage_mv));
		raw.append(" is_read:"); raw.append(std::to_string(is_read));
		raw.append(" channel:"); raw.append(std::to_string(channel));
		return raw;
	};
};

class DeviceDriverSourceCurrentLimit :public VisaDriverIoctrlBase
{
protected:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::SourceCurrentLimit;
public:
	bool is_read = true;
	int32_t current_ma = 0;
	int32_t channel = -1;
	VisaDriverIoctrl GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(result);
		raw.append(" current_ma:"); raw.append(std::to_string(current_ma));
		raw.append(" is_read:"); raw.append(std::to_string(is_read));
		raw.append(" channel:"); raw.append(std::to_string(channel));
		return raw;
	};
};
//ReadQuery
class DeviceDriverReadQuery :public VisaDriverIoctrlBase
{
public:
	enum class QueryMeasFunc {
		MeasDCV,
		MeasACV,
		MeasDCI,
		MeasACI,
	};
protected:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::ReadQuery;
public:
	QueryMeasFunc mMeasfunc= QueryMeasFunc::MeasDCV;
	double current_ma = 0xffffffff;
	double voltage_mv = 0;
	double ativetime_ms = 0;
	int32_t channel = -1;
	VisaDriverIoctrl GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append(" commond:"); raw.append(commond);
		raw.append(" result:"); raw.append(result);
		raw.append(" current_ma:"); raw.append(std::to_string(current_ma));
		raw.append(" voltage_mv:"); raw.append(std::to_string(voltage_mv));
		raw.append(" ativetime_ms:"); raw.append(std::to_string(ativetime_ms));
		raw.append(" channel:"); raw.append(std::to_string(channel));
		return raw;
	};
	QString ShowStatus() {
		QString raw = "";
		QString vstr = QString::asprintf("%0.3lf", voltage_mv > 1000 ? (voltage_mv / 1000) : voltage_mv);
		QString vunint = voltage_mv > 1000 ? "V" : "mV";

		QString Istr = QString::asprintf("%0.3lf", current_ma > 1000 ? (current_ma / 1000) : current_ma);
		QString Iunint = current_ma > 1000 ? "A" : "mA";
		if (channel != -1)
			raw.append(QString(" CH%1").arg(channel));
		if (0xffffffff != current_ma)
			raw.append(QString("V: %1 %2	I: %3 %4").arg(vstr).arg(vunint).arg(Istr).arg(Iunint));
		else
			raw.append(QString("V: %1 %2").arg(vstr).arg(vunint));
		return raw;

	}
};
class DeviceDriverRelayChannelRW :public VisaDriverIoctrlBase
{
protected:
	VisaDriverIoctrl cmd = VisaDriverIoctrl::RelayChannelRW;
public:
	VisaDriverIoctrl GetCmd() { return cmd; };
	std::string to_string() {
		std::string raw;
		raw.append("Cmd:"); raw.append(std::to_string(int(cmd)));
		raw.append("read: "); raw.append(std::to_string(is_read));
		{
			raw.append(" channelmask: "); raw.append(Utility::ShortIntToBrinaryString(channelmask).toStdString());
		}
		{
		raw.append(" channelvalue: "); raw.append(Utility::ShortIntToBrinaryString(channelvalue).toStdString());
		}
		return raw;
	};
	bool is_read = true;
	int32_t channelmask = 0x0;
	int32_t channelvalue = 0x0;

};