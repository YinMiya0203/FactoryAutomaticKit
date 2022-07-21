#include "NaturalLang.h"
#define PURE_FLOAT_RE "-?\\d{1,}(\\.[0-9]+)?"
#define PURE_UINT_RE "\\d+"
#include <QRegularExpression> 
IniInfoBasePtr NaturalLang::translation_slash_smart(std::string input, QString& output, caseitem_class type)
{
	QString input_str = QString(input.c_str());
	output.clear();
	IniInfoBasePtr ptr = nullptr;
	//int slash_cnt = input_str.count('/');

	if (type == caseitem_class::Precondition) {
		ptr = PreconditionWithNetworkId(input_str, output);
		goto ERROR_OUT;
	}
	else if ((type == caseitem_class::Passcondition) ||
		(type == caseitem_class::ManualConfirm)) {
		if (type == caseitem_class::ManualConfirm && input_str.contains("Resource:", Qt::CaseInsensitive)) {
			ptr = ManualConfirmWithRes(input_str, output);
			goto ERROR_OUT;
		}
		else {
			ptr = PassconditionWithNetworkId(input_str, output);
			goto ERROR_OUT;
		}
	}
	else if (type == caseitem_class::ManualNotice) {
		ptr = ManualConfirmWithRes(input_str, output);
		goto ERROR_OUT;
	}
	if (output.size() == 0) {
		auto out = input_str.split('/');
		foreach(auto v, out) {
			output.append(v);
			output.append(' ');
		}
	}
ERROR_OUT:
	return ptr;
}
IniInfoBasePtr NaturalLang::PreconditionWithNetworkId(QString input_str, QString& output)
{
	IniInfoBasePtr ptr = nullptr;
	QRegularExpression re("(?<networklabel>\\w+)/(?<voltage>" PURE_FLOAT_RE ")(?<voltageunit>\\w+)/(?<duration>" PURE_UINT_RE ")(?<durationunit>\\w+)");
	//QRegularExpression re("(?<networklabel>\\w+)/(?<voltage>\\d+)(?<voltageunit>\\w+)/(?<duration>\\d+)(?<durationunit>\\w+)");
	//^-?\\d{1,}(\\.[0-9]+)?
	QRegularExpressionMatch match = re.match(input_str);
	if (match.hasMatch()) {
		auto info = new NetworkLabelPreconditionBase;
		info->networklabel = match.captured("networklabel");
		{
			float voltage_mv = match.captured("voltageunit").toUpper().compare("MV") == 0 ? 1 : 1000;
			//qDebug("voltage_mv %f %s", match.captured("voltage").toFloat(), match.captured("voltage").toStdString().c_str());
			voltage_mv *= match.captured("voltage").toFloat();
			//qDebug("voltage_mv %f", voltage_mv);
			info->voltage_mv = voltage_mv;
		}
		{
			int duration_ms = match.captured("durationunit").toUpper().compare("MS") == 0 ? 1 : 1000;
			duration_ms *= match.captured("duration").toFloat();
			info->duration_ms = duration_ms;
		}
		ptr = IniInfoBasePtr(info);
		output = info->to_string();
		goto ERROR_OUT;
	}
	else {
		qCritical("input %s match fail", input_str.toStdString().c_str());
	}
ERROR_OUT:
	return ptr;
}
IniInfoBasePtr NaturalLang::ManualConfirmWithRes(QString input_str, QString& output)
{
	IniInfoBasePtr ptr = nullptr;
	auto info = new ManualViewInfoBase;
	auto raw_list = input_str.split('/');
	foreach(auto str, raw_list) {
		if (str.contains("Resource:", Qt::CaseInsensitive)) {
			info->resource.push_back(str.right(str.size() - QString("Resource:").size()));
		}
		else if (str.contains(QRegExp("^\\d+$"))) {
			info->duration_ms = str.toInt();
		}
		else {
			info->msgs.push_back(str);
		}
	}
	ptr = IniInfoBasePtr(info);
	output = info->to_string();
	goto ERROR_OUT;
ERROR_OUT:
	return ptr;
}
IniInfoBasePtr NaturalLang::PassconditionWithNetworkId(QString input_str, QString& output)
{
	IniInfoBasePtr ptr = nullptr;
	//QRegularExpression re("(?<networklabel>\\w+)/(?<mode>\\w+)/[(](?<rangmin>" PURE_FLOAT_RE ")(?<rangminunit>\\w+)/(?<rangmax>" PURE_FLOAT_RE ")(?<rangmaxunit>\\w+)[)]/(?<duration>" PURE_UINT_RE ")(?<durationunit>\\w+)");
	QRegularExpression re("(?<networklabel>\\w+)/"
		"(?<mode>\\w+)/"
		"[(](?<rangmin>" PURE_FLOAT_RE ")(?<rangminunit>\\w+)[|]?(?<variance_min>" PURE_FLOAT_RE ")?/"
		"(?<rangmax>" PURE_FLOAT_RE ")(?<rangmaxunit>\\w+)[|]?(?<variance_max>" PURE_FLOAT_RE ")?[)]/"
		"(?<duration>" PURE_UINT_RE ")(?<durationunit>\\w+)");
	QRegularExpressionMatch match = re.match(input_str);
	if (match.hasMatch()) {
		auto info = new NetworkLabelPassconditionBase;
		info->networklabel = match.captured("networklabel");
		info->mode = match.captured("mode");
		{
			int uintmin = 1000, uintmax = 1000;

			QString rangminunit = match.captured("rangminunit");
			QString rangmaxunit = match.captured("rangmaxunit");
			QString variance_min = match.captured("variance_min");
			auto variance_minv = variance_min.toFloat();
			QString variance_max = match.captured("variance_max");
			auto variance_maxv = variance_max.toFloat();
			//qDebug("rangminunit %s ", rangminunit.toStdString().c_str());
			//qDebug("rangmaxunit %s ", rangmaxunit.toStdString().c_str());

			if (rangminunit.toUpper().count("M") != 0) {
				//V
				uintmin = 1;

			}
			if (rangmaxunit.toUpper().count("M") != 0) {
				//V
				uintmax = 1;
			}
			QString unit;
			if (uintmax < uintmin) {
				unit = rangmaxunit;
			}
			else {
				unit = rangminunit;
			}
#if 1
			if (uintmin == 1000 && uintmax == 1000) {
				if (rangmaxunit.toUpper().count("V") != 0) {
					unit = "mV";
				}
				else {
					unit = "mA";
				}
			}
#endif
			info->rangmin_m = match.captured("rangmin").toDouble() * uintmin * (1 + variance_minv);
			info->rangmax_m = match.captured("rangmax").toDouble() * uintmax * (1 + variance_maxv);
			info->unit = unit;
			//qDebug("rangmin %s %lf ", match.captured("rangmin").toStdString().c_str(), info->rangmin_m);
			//qDebug("rangmax %s %lf unit[%s]", match.captured("rangmax").toStdString().c_str(), info->rangmax_m, unit.toStdString().c_str());

		}
		{
			auto duration_str = match.captured("durationunit").toUpper().split('_');
			int duration_ms = duration_str[0].compare("MS") == 0 ? 1 : 1000;
			duration_ms *= match.captured("duration").toInt();
			info->duration_ms = duration_ms;
			info->keep_wait_ability = false;
			if (duration_str.size() > 1 && duration_str[1].compare("W") == 0) {
				info->keep_wait_ability = true;
			}
			//qDebug("duration_ms %d [%s][%s]", duration_ms, match.captured("duration").toStdString().c_str(),match.captured("durationunit").toStdString().c_str());

		}

		ptr = IniInfoBasePtr(info);
		output = info->to_string();
		goto ERROR_OUT;
	}
	else {
		if (input_str.contains("/"))qCritical("input %s match fail", input_str.toStdString().c_str());
	}
ERROR_OUT:
	return ptr;
}