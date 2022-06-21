#pragma once
#include <cstdint>
#include <string>
#include <visa.h>
#include <QDebug>
#include <QMutex>
#include "winerror.h"
#include <exception>
#include "GlobalConfig.h"
#include "VisaDriver_ioctrl.h"
#include "Utility.h"
#include <memory>
#define NIDD_VERSION_M	"0"
#define NIDD_VERSION_S	"1"
#define NIDD_VERSION_P	"2"
#define NIDD_VERSION NIDD_VERSION_M "." NIDD_VERSION_S "." NIDD_VERSION_P
typedef std::list<std::string> Resourcecontainer;
typedef struct asrlconfg_t {
	uint32_t baud_rate = 9600;
	uint8_t data_bits = 8;
	uint8_t parity = VI_ASRL_PAR_NONE;
	uint8_t stop_bits = 2;
	uint8_t flow_control = VI_ASRL_FLOW_NONE;
public:
	QString to_string() {
		return QString::asprintf("Set Attribute %d/%d/%d/%d/%d", baud_rate, data_bits, stop_bits,
			parity, flow_control);
	};
}xxx;
enum class DriverClass {
	DriverSCPI,
	DriverDMMIVictor,
};
class NiDeviceDriverBase;
typedef std::shared_ptr<NiDeviceDriverBase> NiDeviceDriverBasePtr;
class NiDeviceDriverBase
{
public:
	explicit NiDeviceDriverBase();
	~NiDeviceDriverBase();
public:
	int32_t FindDeviceResource(Resourcecontainer& contain);
	bool isResourceVaild(std::string res);
	int32_t ioctrl(VisaDriverIoctrlBasePtr ptr = nullptr);
	int32_t Driveropen(std::string res);
	virtual int32_t Driversetattribute(asrlconfg_t config);
	virtual int32_t Driverclose();
	virtual std::string GetCmdPostfix();
	void SetIndexInList(int value);
	void SetCmdPostfix(std::string val) {
		mCmdPostfix = val;
	}
	static QString GetVersion() {

		return QString("%1 [%2]").arg("NIDD").arg(NIDD_VERSION);
	};
protected:
	int32_t write(VisaDriverIoctrlWrite* arg);
	int32_t read(VisaDriverIoctrlRead* arg);
	std::string mCmdPostfix = "";
	QMutex mdrivermutex;
	int32_t moffset_inlist = -1;
	ViSession vi = VI_NULL;
	ViSession rm = VI_NULL;
};