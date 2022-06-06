#include "VisaDriver.h"
#include "winerror.h"
#include <exception>
#if 0
int32_t VisaDriver::Driveropen(std::string res)
{
    int ret = 0;
    ViStatus status=VI_SUCCESS;

    if (vi) {
        qDebug("had opened");
        goto ERR_OUT;
    }
    if (GlobalConfig_debugdevciedriver)qDebug("res %s", res.c_str());
#ifdef VIRTUAL_DEVICE
    return ret;
#endif  
    {
        
        if (rm == VI_NULL) { 
            status = viOpenDefaultRM(&rm); 
        }
        else { 
            status = VI_SUCCESS; 
        }
        if (status<VI_SUCCESS) {
            ret = -ERROR_PATH_NOT_FOUND;
            goto ERR_OUT;
        }
        if (!isResourceVaild(res)) {
            ret = -ERROR_PATH_NOT_FOUND;
            goto ERR_OUT;
        }
        status = viOpen(rm, res.c_str(), VI_NULL, VI_NULL, &vi);
        if (status < VI_SUCCESS) {
            ret = -ERROR_PATH_NOT_FOUND;
            goto ERR_OUT;
        }
    }
ERR_OUT:
    qDebug("ret %d",ret);
    if (ret != 0) {
        Driverclose();
    }
    return ret;
}
#endif
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
        qDebug("close vi");
        viClose(vi); 
        vi = VI_NULL;
    }
    if (rm != VI_NULL) {
        qDebug("close rm");
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
