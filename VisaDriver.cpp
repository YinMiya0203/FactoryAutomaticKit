#include "VisaDriver.h"
#include "winerror.h"
#include <exception>

int32_t VisaDriver::Driverclose()
{
    int32_t ret = 0;
#ifdef VIRTUAL_DEVICE
    return ret;
#endif
    if (vi==VI_NULL || rm== VI_NULL) {
        //ret = -ERROR_PATH_NOT_FOUND;
        //printf("vi or rm not init");
    }
    if (vi != VI_NULL) { 
        if (GlobalConfig_debugdevciedriver)qDebug("close vi");
        viClose(vi); 
        vi = VI_NULL;
    }
    if (rm != VI_NULL) {
        if (GlobalConfig_debugdevciedriver)qDebug("close rm");
        viClose(rm);
        rm = VI_NULL;
    }
    return ret;
}

int32_t VisaDriver::Driversetattribute(const asrlconfg_t config)
{
    int ret = 0;
    ViStatus status=VI_SUCCESS;
    ret = NiDeviceDriverBase::Driversetattribute(config);
    if (ret == 0) {
        status |= viSetAttribute(vi, VI_ATTR_ASRL_END_OUT, VI_ASRL_END_TERMCHAR);
        status |= viSetAttribute(vi, VI_ATTR_TERMCHAR_EN, 1);
        if (status != VI_SUCCESS) {
            ret = -ERROR_INVALID_DATA;
        }
    }
    return ret ;
}
