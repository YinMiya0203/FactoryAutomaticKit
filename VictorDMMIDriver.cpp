#include "VictorDMMIDriver.h"
#include "VisaDriver.h"
#include "winerror.h"
#include <exception>
int32_t VictorDMMIDriver::Driversetattribute(asrlconfg_t config)
{
    int ret = 0;
    ViStatus status = VI_SUCCESS;
    if(GlobalConfig_debugdevciedriver)qDebug(" ");
    ret = NiDeviceDriverBase::Driversetattribute(config);
    if (ret == 0) {
        //status |= viSetAttribute(vi, VI_ATTR_ASRL_END_OUT, VI_ASRL_END_TERMCHAR);
        status |= viSetAttribute(vi, VI_ATTR_TMO_VALUE, 4000);
        status |= viSetAttribute(vi, VI_ATTR_TERMCHAR_EN, 1);
        status |= viSetAttribute(vi, VI_ATTR_TERMCHAR, 0x0a);
        if (status != VI_SUCCESS) {
            ret = status;
        }else
        {
            ret = device_online();
        }
    }
    else {
        qCritical("NiDeviceDriverBase::Driversetattribute %s fail", config.to_string().toStdString().c_str());
    }
ERROR_OUT:
    return ret;
}

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

int32_t VictorDMMIDriver::device_rst()
{
    int ret = 0;
    VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
    mptr->commond = "#*RST";
    ret = ioctrl(mptr);
    //可能有ack，可能没有
    if (ret == -ERROR_TIMEOUT) {
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
        Sleep(3*1000);//wait rst
        {
            VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
            mptr->commond = "#*ONL";
            ret = ioctrl(mptr);
            //由于rst后必须有ack
            if (ret == 0) {
                Sleep(1 * 1000);//wait rst
                if (mptr->result.size()==0) {
                    ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
                }
            }
            Sleep(1 * 1000);//wait ONL
        }
    } while (0);
    return ret;
}
