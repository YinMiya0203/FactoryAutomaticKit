#pragma once
#include "NIDeviceDriverBase.h"
class RelayBMWDriver :
    public NiDeviceDriverBase
{
public:
    //int32_t write(VisaDriverIoctrlWrite* arg);
    int32_t read(VisaDriverIoctrlRead* arg);
    int32_t Driversetattribute(const asrlconfg_t config);
public:
    static int32_t ReadChannelMaskString(std::string & output, int max_channel);
    static int32_t WriteChannelMaskString(std::string& output,int32_t channel_mask,int32_t value, int max_channel);

    static int32_t Local8bitToString(const uint8_t *buf, int buf_size, QStringList& output);
    static int32_t Local8bitToString(const uint8_t* buf, int buf_size, std::string & output);
    static int32_t StringToLocal8bit(std::string input, uint8_t* buff,int buff_size);
    static int32_t StringToLocal8bit(QStringList input, uint8_t* buff, int buff_size);
    static int32_t LongFormatStringToShortInt(const uint8_t* buf, int buf_size, int32_t& output,int pos=2,int len=8);
    static int32_t LongFormatStringToShortInt(std::string input, int buf_size, int32_t& output, int pos = 2, int len = 8);

};

