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
typedef std::list<std::string> Resourcecontainer;
typedef struct asrlconfg_t {
	uint32_t baud_rate = 9600;
	uint8_t data_bits = 8;
	uint8_t parity = VI_ASRL_PAR_NONE;
	uint8_t stop_bits = 2;
	uint8_t flow_control = VI_ASRL_FLOW_NONE;
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
	virtual int32_t Driversetattribute(const asrlconfg_t config);
	virtual int32_t Driverclose();
	virtual std::string GetCmdPostfix();
	void SetIndexInList(int value);
	void SetCmdPostfix(std::string val) {
		mCmdPostfix = val;
	}
protected:
	int32_t write(VisaDriverIoctrlWrite* arg);
	int32_t read(VisaDriverIoctrlRead* arg);
	std::string mCmdPostfix = "";
	QMutex mdrivermutex;
	int32_t moffset_inlist = -1;
	ViSession vi = VI_NULL;
	ViSession rm = VI_NULL;
};