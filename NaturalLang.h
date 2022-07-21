#pragma once
#include "CaseItemBase.h"
class NaturalLang
{
public:
	static IniInfoBasePtr translation_slash_smart(std::string input, QString& output, caseitem_class type = caseitem_class::Unknown);
	static IniInfoBasePtr PassconditionWithNetworkId(QString input, QString& output);
	static IniInfoBasePtr ManualConfirmWithRes(QString input, QString& output);
	static IniInfoBasePtr PreconditionWithNetworkId(QString input, QString& output);
};

