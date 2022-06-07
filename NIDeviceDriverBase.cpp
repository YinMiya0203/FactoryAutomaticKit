#include "NIDeviceDriverBase.h"
#include "NiDeviceDriverBase.h"
#include "winerror.h"
#include <exception>

NiDeviceDriverBase::NiDeviceDriverBase() :vi(VI_NULL), rm(VI_NULL)
{
}

NiDeviceDriverBase::~NiDeviceDriverBase()
{
    Utility::OutputDebugPrintf("~NiDeviceDriverBase %d\r\n", moffset_inlist);
    Driverclose();
}

void NiDeviceDriverBase::SetIndexInList(int value) {
    moffset_inlist = value;
}
int32_t NiDeviceDriverBase::FindDeviceResource(Resourcecontainer& contain)
{
    int ret = 0;
    char* res = NULL;
    ViStatus status;
    ViFindList flist;
    ViUInt32 itemCnt=0;
    ViChar desc[256];
    contain.clear();
#ifdef VIRTUAL_DEVICE
    _sleep(3 * 1000);
    contain.push_back("TCPIP::169.254.247.189::INSTR");
    contain.push_back("COM3::INSTR");
    return ret;
#endif
    if (rm == NULL) {
        status = viOpenDefaultRM(&rm);
    }
    else {
        status = VI_SUCCESS;
    }
    if (rm == NULL) {
        qCritical("viOpenDefaultRM fail");
        ret = -ERROR_ENVVAR_NOT_FOUND;
        goto error;
    }
    status = viFindRsrc(rm, "?*INSTR", &flist, &itemCnt, desc);
    if (status < VI_SUCCESS || itemCnt <= 0) {
        qCritical("dump status 0x%x itemCnt %u\n", status, itemCnt);
        ret = -1;
        goto error;
    }
    for (ViUInt32 i = 0; i < itemCnt; i++) {
        res = desc;
        qInfo("desc %s \n", desc);
        contain.push_back(std::string(res));
        viFindNext(flist, desc);
    }
error:
    return ret;
}

bool NiDeviceDriverBase::isResourceVaild(std::string res)
{
    bool ret = false;
    Resourcecontainer contain;
    if (FindDeviceResource(contain) == 0 && std::find(contain.begin(), contain.end(), res) != contain.end()) {
        ret = true;
    }
    return ret;
}
int32_t NiDeviceDriverBase::Driversetattribute(const asrlconfg_t config)
{
    int ret = 0;
    ViStatus status;
#ifdef VIRTUAL_DEVICE
    return ret;
#endif
    if(GlobalConfig_debugdevciedriver)qDebug("vi %p", vi);
    if (vi == VI_NULL || rm == VI_NULL) {
        ret = -ERROR_PATH_NOT_FOUND;
        qCritical("driver not open");
        goto ERR_OUT;
    }
    if (GlobalConfig_debugdevciedriver)qDebug("Set Attribute %d/%d/%d/%d/%d", config.baud_rate, config.data_bits, config.stop_bits,
        config.parity, config.flow_control);
    status = viSetAttribute(vi, VI_ATTR_ASRL_BAUD, config.baud_rate);
    status |= viSetAttribute(vi, VI_ATTR_ASRL_DATA_BITS, config.data_bits);
    status |= viSetAttribute(vi, VI_ATTR_ASRL_END_IN, config.stop_bits);
    status |= viSetAttribute(vi, VI_ATTR_ASRL_PARITY, config.parity);
    status |= viSetAttribute(vi, VI_ATTR_ASRL_FLOW_CNTRL, config.flow_control);
    //status |= viSetAttribute(vi, VI_ATTR_TMO_VALUE, 4000);
    //status |= viSetAttribute(vi, VI_ATTR_ASRL_END_OUT, VI_ASRL_END_TERMCHAR);
    //status |= viSetAttribute(vi, VI_ATTR_TERMCHAR_EN, 1);
    if (status != VI_SUCCESS) {
        ret = -ERROR_INVALID_DATA;
    }
ERR_OUT:
    return ret;
}
int32_t NiDeviceDriverBase::Driveropen(std::string res)
{
    int ret = 0;
    ViStatus status = VI_SUCCESS;

    if (vi) {
        qCritical("had opened");
        status = VI_ERROR_FILE_ACCESS;
        ret = -ERROR_DEVICE_IN_USE;
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
    if (GlobalConfig_debugdevciedriver)qDebug("ret %d", ret);
    if (ret != 0) {
        Driverclose();
    }
    return ret;
}

int32_t NiDeviceDriverBase::Driverclose()
{
    int32_t ret = 0;
    OutputDebugStringA("NiDeviceDriverBase::Driverclose()");
#ifdef VIRTUAL_DEVICE
    return ret;
#endif
    if (vi == VI_NULL || rm == VI_NULL) {
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

std::string NiDeviceDriverBase::GetCmdPostfix()
{
    return mCmdPostfix;
}

int32_t NiDeviceDriverBase::write(VisaDriverIoctrlWrite* arg)
{
    QMutexLocker locker(&mdrivermutex);
    int32_t ret = 0;
    if (arg == nullptr)return -ERROR_INVALID_PARAMETER;
    if (GlobalConfig_debugdevciedriver)qDebug("index %d command [%s%s]", moffset_inlist, arg->commond.c_str(), GetCmdPostfix().c_str());
#ifdef VIRTUAL_DEVICE
    return ret;
#endif
    if (vi == VI_NULL || rm == VI_NULL) {
        ret = -ERROR_DEVICE_NOT_CONNECTED;
        qCritical("driver not open");
        goto ERR_OUT;
    }
    {
        ViChar cmdbuffer[128];
        ViUInt32 retCnt;
        ViStatus status;
        snprintf(cmdbuffer, sizeof(cmdbuffer), "%s%s",arg->commond.c_str(), GetCmdPostfix().c_str());
        status = viWrite(vi, (ViBuf)cmdbuffer, strlen(cmdbuffer), &retCnt);
        if (status < VI_SUCCESS) {
            ret = status;
            goto ERR_OUT;
        }
    }
ERR_OUT:
    return ret;
}
int32_t NiDeviceDriverBase::read(VisaDriverIoctrlRead* arg)
{
    QMutexLocker locker(&mdrivermutex);
    int32_t ret = 0;
    arg->result.clear();
    if (arg == nullptr)return -ERROR_INVALID_PARAMETER;
    if (GlobalConfig_debugdevciedriver)qDebug("index %d command [%s%s]", moffset_inlist, arg->commond.c_str(), GetCmdPostfix().c_str());
#ifdef VIRTUAL_DEVICE
    return ret;
#endif
    if (vi == VI_NULL || rm == VI_NULL) {
        ret = -ERROR_DEVICE_NOT_CONNECTED;
        qCritical("driver not open");
        goto ERR_OUT;
    }
    {
        ViChar cmdbuffer[128];
        ViUInt32 retCnt;
        ViStatus status;
        snprintf(cmdbuffer, sizeof(cmdbuffer), "%s%s", arg->commond.c_str(), GetCmdPostfix().c_str());
        status = viWrite(vi, (ViBuf)cmdbuffer, strlen(cmdbuffer), &retCnt);
        if (status < VI_SUCCESS) goto ERR_OUT;
        memset(cmdbuffer, 0, sizeof(cmdbuffer));
        status = viRead(vi, (ViBuf)cmdbuffer, sizeof(cmdbuffer), &retCnt);
        if (status < VI_SUCCESS) {
            ret = status;
            goto ERR_OUT;
        }
        arg->result = QString(cmdbuffer).trimmed().toStdString();
    }
ERR_OUT:
    if (ret != 0) {
        qCritical("ret 0x%x", ret);
    }
    return ret;
}
int32_t NiDeviceDriverBase::ioctrl(VisaDriverIoctrlBasePtr ptr)
{
    int ret = 0;
    VisaDriverIoctrl cmd = ptr->GetCmd();
    switch (cmd)
    {
    case VisaDriverIoctrl::IoRead:
    {
        VisaDriverIoctrlRead* arg = dynamic_cast<VisaDriverIoctrlRead*>(ptr.get());
        if (arg == nullptr || arg->commond.size() <= 0) {
            ret = -ERROR_INVALID_PARAMETER;
            break;
        }
        else {
            ret = read(arg);
        }
    }
    break;
    case VisaDriverIoctrl::IoWrite:
    {
        VisaDriverIoctrlWrite* arg = dynamic_cast<VisaDriverIoctrlWrite*>(ptr.get());
        if (arg == nullptr || arg->commond.size() <= 0) {
            ret = -ERROR_INVALID_PARAMETER;
            break;
        }
        else {
            ret = write(arg);
        }
    }
    break;
    default:
        ret = -VI_ERROR_INV_PARAMETER;
        break;
    }
    if (ret != 0) {
        qCritical("ret %d cmd %d", ret, cmd);
    }
    return ret;
}

