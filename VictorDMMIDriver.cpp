#include "VictorDMMIDriver.h"
#include "VisaDriver.h"
#include "winerror.h"
#include <exception>
int32_t VictorDMMIDriver::Driversetattribute(const asrlconfg_t config)
{
    int ret = 0;
    ViStatus status = VI_SUCCESS;
    if(GlobalConfig_debugdevciedriver)qDebug(" ");
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
    //������ack������û��
    if (ret == -ERROR_TIMEOUT) {
        ret = 0;
    }
    return ret;
}

int32_t VictorDMMIDriver::device_online()
{
    //�����豸״̬δ֪��rstһ��
    int ret = 0;
    do {
        ret = device_rst();
        Sleep(5*1000);//wait rst
        {
            VisaDriverIoctrlBasePtr mptr(new VisaDriverIoctrlRead);
            mptr->commond = "#*ONL";
            ret = ioctrl(mptr);
            //����rst�������ack
            if (ret == 0) {
                Sleep(1 * 1000);//wait rst
                if (mptr->result.size()==0) {
                    ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
                }
            }
        }
    } while (0);
    return ret;
}
