#include "RelayBMWDriver.h"

int32_t RelayBMWDriver::read(VisaDriverIoctrlRead* arg)
{
    QMutexLocker locker(&mdrivermutex);
    int32_t ret = 0;
    arg->result.clear();
    if (arg == nullptr)return -ERROR_INVALID_PARAMETER;
    if (GlobalConfig_debugdevciedriver) {
        qInfo("index %d command [%s%s]", moffset_inlist, arg->commond.c_str(), GetCmdPostfix().c_str());
    }
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
        int cmdsize = StringToLocal8bit(arg->commond, (uint8_t*)cmdbuffer,sizeof(cmdbuffer));
        //Utility::DumpHex((uint8_t*)cmdbuffer, cmdsize, "cmdbuffer");
        status = viWrite(vi, (ViBuf)cmdbuffer, cmdsize, &retCnt);
        if (status != VI_SUCCESS) {
            ViChar		mdescbuffer[64] = { 0 };
            viStatusDesc(vi, status, mdescbuffer);
            qInfo("index %d command [%s%s]", moffset_inlist, arg->commond.c_str(), GetCmdPostfix().c_str());
            qCritical("Desc 0x%x [%s]", status, QString::fromLocal8Bit(mdescbuffer).toStdString().c_str());
            if (status == VI_ERROR_TMO)ret = -ERROR_TIMEOUT;
            goto ERR_OUT;
        }
        memset(cmdbuffer, 0, sizeof(cmdbuffer));
        Utility::Sleep(1000);//sleep 后usb bus 数据完整
        //bmw read 固定大小回复
        {
            retCnt = 0;
            int total_retcnt = 0;
            int try_cnt = 0;
            while (total_retcnt < cmdsize) {    //bmw8008 首个read cnt 常为1，所以循环读取
                if (try_cnt> cmdsize) {
                    qCritical("Try read % bytes had try %d times", cmdsize, try_cnt);
                    ret = -ERROR_DEVICE_HARDWARE_ERROR;
                    goto ERR_OUT;
                }
                status = viRead(vi, (ViBuf)cmdbuffer+ total_retcnt, cmdsize- total_retcnt, &retCnt);
                //qDebug("viread retCnt %d", retCnt);
                if (status != VI_SUCCESS) {
                    ViChar		mdescbuffer[64] = { 0 };
                    viStatusDesc(vi, status, mdescbuffer);
                    qInfo("index %d command [%s%s]", moffset_inlist, arg->commond.c_str(), GetCmdPostfix().c_str());
                    qCritical("Desc 0x%x [%s]", status, QString::fromLocal8Bit(mdescbuffer).toStdString().c_str());

                    if (status == VI_ERROR_TMO)ret = -ERROR_TIMEOUT;
                    goto ERR_OUT;
                }
                total_retcnt += retCnt;
                try_cnt++;
            }
        }
        //arg->result = QString(cmdbuffer).trimmed().toStdString();
        //Utility::DumpHex((uint8_t*)cmdbuffer, cmdsize, "viRead cmdbuffer");
        Local8bitToString((uint8_t*)cmdbuffer, cmdsize, arg->result);
        if (GlobalConfig_debugdevciedriver) {
            qInfo("index %d command [%s%s]", moffset_inlist, arg->result.c_str(), GetCmdPostfix().c_str());
        }
    }
ERR_OUT:
    if (ret != 0) {
        qDebug("ret 0x%x", ret);
    }
    return ret;
}
int32_t RelayBMWDriver::Driversetattribute(const asrlconfg_t config)
{
    int ret = 0;
    ViStatus status = VI_SUCCESS;
    ret = NiDeviceDriverBase::Driversetattribute(config);
    if (ret == 0) {
        status |= viSetAttribute(vi, VI_ATTR_TERMCHAR_EN, 0);
        status |= viSetAttribute(vi, VI_ATTR_TMO_VALUE, 4000);
        if (status != VI_SUCCESS) {
            ret = -ERROR_INVALID_DATA;
        }
    }
    return ret;
}
//kit
int32_t RelayBMWDriver::ReadChannelMaskString(std::string& output, int max_channel)
{
    output.clear();
    uint8_t buff[64];
    memset(buff, 0x0, sizeof(buff));
    buff[0] = 0xfd;
    buff[1] = 0x02;
    buff[2+max_channel] = 0xdf;
    int ret = Local8bitToString(buff, max_channel+3, output);
    if (GlobalConfig_debugdevciedriver) {
        qDebug("Comand %d [%s]", ret, output.c_str());
    }
    return ret;
}

int32_t RelayBMWDriver::WriteChannelMaskString(std::string& output, int32_t channel_mask, int32_t value, int max_channel)
{
    output.clear();
    uint8_t buff[64];
    memset(buff, 0x0, sizeof(buff));
    buff[0] = 0xfd;
    buff[1] = 0x01;
    int pos = 2;
    buff[pos + max_channel] = 0xdf;
    QString bchannel_mask = Utility::ShortIntToBrinaryString(channel_mask);
    QString bvalue = Utility::ShortIntToBrinaryString(value);
    for (int index = 0; index < max_channel;index++) {
        int tmp_value = 0xff;//invaild
        if (bchannel_mask.at(index)=="1") {
            tmp_value = bvalue.at(index)=="1"?1:0;
        }
        buff[pos + index] = tmp_value;
    }

    return Local8bitToString(buff, max_channel + 3, output);
}

int32_t RelayBMWDriver::Local8bitToString(const uint8_t* buf, int buf_size, QStringList& output)
{
    output.clear();
    for (int index = 0; index < buf_size; index++) {
        output.append(QString::asprintf("%02x",buf[index]));
    }
    return buf_size;
}

int32_t RelayBMWDriver::Local8bitToString(const uint8_t* buf, int buf_size, std::string& output)
{
    QStringList tmp;
    output.clear();
    int ret = Local8bitToString(buf, buf_size, tmp);
    output = tmp.join(" ").toStdString();
    return ret;
}

int32_t RelayBMWDriver::StringToLocal8bit(std::string input, uint8_t* buff, int buff_size)
{
    //
    return StringToLocal8bit(
        QString(input.c_str()).split(" "),
        buff,buff_size);
}

int32_t RelayBMWDriver::StringToLocal8bit(QStringList input, uint8_t* buff, int buff_size)
{
    int total = input.size();
    //qDebug("listinput %d", total);
    memset(buff,0, buff_size);
    //逆序
    for (int i = 0; (i < total)&&(i< buff_size); i++) {
        bool is_ok = false;
        buff[i] = input.at(i).toInt(&is_ok, 16);
        if (!is_ok) {
            qCritical(" %d/%d [%s] tohex fail",i,total, input.at(total - i).toStdString().c_str());
        }
    }
    return input.size();
}

int32_t RelayBMWDriver::LongFormatStringToShortInt(const uint8_t* buf,int buf_size, int32_t& output, int pos, int len)
{
    int ret = 0;
    output = 0;
    if ((pos+len) >= buf_size) {
        qCritical(" offset  %d out of %d",pos+len,buf_size);
        ret = -ERROR_OUT_OF_PAPER;
        goto ERROR_OUT;
    }
    for (int index = 0; index < len;index++) {
        output = output << 1;
        int value = buf[pos + index]==1;
        output |= value;
    }
ERROR_OUT:
    return ret;
}

int32_t RelayBMWDriver::LongFormatStringToShortInt(std::string input, int buf_size, int32_t& output, int pos, int len)
{
    uint8_t cmdbuffer[64];
    int result_cn = StringToLocal8bit(input, cmdbuffer, sizeof(cmdbuffer));
    LongFormatStringToShortInt(cmdbuffer, sizeof(cmdbuffer), output);
    return result_cn;
}





