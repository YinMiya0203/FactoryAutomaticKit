#pragma once
#include "CaseItemBase.h"
#define CHANNELTOMASK(x)	(0x1 << (x-1))
#define CHANNEVALUELTOMASK(v,c)	((!!v) << (c-1))
class NaturalLang
{
public:
	static IniInfoBasePtr translation_slash_smart(std::string input, QString& output, caseitem_class type = caseitem_class::Unknown);
	static IniInfoBasePtr PassconditionWithNetworkId(QString input, QString& output);
	static IniInfoBasePtr ManualConfirmWithRes(QString input, QString& output);
	static IniInfoBasePtr PreconditionWithNetworkId(QString input, QString& output);
	static int NetworkLabelChannel(QString val);
	static int32_t ChannelMaskStringToInt(int32_t& output, QString input);
	static int32_t ChannelValueStringToInt(int32_t& output, QString channelmask_str, int32_t channelmask, QString value);
	
};

