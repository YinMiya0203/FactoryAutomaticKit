#include "VictorDMMIDriver.h"
#include "VisaDriver.h"
#include "winerror.h"
#include <exception>
int32_t VictorDMMIDriver::Driversetattribute(const asrlconfg_t config)
{
    int ret = 0;
    ViStatus status = VI_SUCCESS;
    qDebug(" ");
    ret = NiDeviceDriverBase::Driversetattribute(config);
    if (ret == 0) {
        //status |= viSetAttribute(vi, VI_ATTR_ASRL_END_OUT, VI_ASRL_END_TERMCHAR);
        status |= viSetAttribute(vi, VI_ATTR_TERMCHAR_EN, 1);
        if (status != VI_SUCCESS) {
            ret = status;
        }
        {
            ret = device_online();
        }
    }
ERROR_OUT:
    return ret;
}
#if 0
int32_t VictorDMMIDriver::Driveropen(std::string res)
{
    int ret = 0;
    ViStatus status = VI_SUCCESS;
    mCmdPostfix = "\r\n";
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
        if (status < VI_SUCCESS) {
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
    qDebug("ret %d", ret);
    if (ret != 0) {
        Driverclose();
    }
    return ret;
}
#endif
int32_t VictorDMMIDriver::Driverclose()
{
    int32_t ret = 0;
#ifdef VIRTUAL_DEVICE
    return ret;
#endif
    if (vi == VI_NULL || rm == VI_NULL) {
        //ret = -ERROR_PATH_NOT_FOUND;
        //printf("vi or rm not init");
    }

    if (vi != VI_NULL) {
        device_rst();
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

int32_t VictorDMMIDriver::device_rst()
{
    int ret = 0;
    VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
    mptr->commond = "#*RST";
    ret = ioctrl(mptr);
    //可能有ack，可能没有
    if (ret == VI_ERROR_TMO) {
        ret = 0;
    }
    return ret;
}

int32_t VictorDMMIDriver::device_online()
{
    //由于设备状态未知先rst一下
    int ret = 0;
    do {
        ret = device_rst();
        Sleep(5*1000);//wait rst
        {
            VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
            mptr->commond = "#*ONL";
            ret = ioctrl(mptr);
            //由于rst后必须有ack
            if (ret == 0) {
                if (mptr->result.size()==0) {
                    ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
                }
            }
        }
    } while (0);
    return ret;
}
