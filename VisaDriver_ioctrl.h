#pragma once
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
	Io_max,
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
	double current_ma = 0;
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
};