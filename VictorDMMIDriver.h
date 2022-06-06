#pragma once
#include "NIDeviceDriverBase.h"
enum class VDMMeasSpanV {
	MV50,
	MV500,
	V5,
	V50,
	V500,
	V1000,
	Vmax,
};
enum class VDMMeasSpanI {
	UA500,
	UA5000,
	MA50,
	MA500,
	A5A,
	A10,
	Amax,
};
enum class VDMeasRate {
	Slow,
	Fast,
};
class VictorDMMIDriver :
    public NiDeviceDriverBase
{
public:
	//int32_t Driveropen(std::string res);
	int32_t Driverclose();
	int32_t Driversetattribute(const asrlconfg_t config);
private:
	int32_t device_online();
	int32_t device_rst();
	bool isonline = false;
};

