#pragma once
#include "NIDeviceDriverBase.h"
//#define VIRTUAL_DEVICE 

class VisaDriver:public NiDeviceDriverBase
{

public:
	//int32_t Driveropen(std::string res);
	int32_t Driverclose();
	int32_t Driversetattribute(const asrlconfg_t config);
};

