#include "Utility.h"
#include <winerror.h>
#include <QProcess>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>

#include <QRegularExpression>
QString Utility::GetWinErrorText(int32_t errcode) {
	int len = 256;
	static CHAR* pmsg = (CHAR*)malloc(len * sizeof(CHAR));
	if (errcode < 0)errcode = -errcode;
	FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errcode,
		//获得缺省语言
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		pmsg,
		len,
		NULL);
	//printf("errcode %x 错误描述：%s", errcode, pmsg);
	return QString::fromLocal8Bit(pmsg);
}
qint64 Utility::GetFileSize_B(QString path)
{
	qint64 total_size =0;
	QDir dir(path);
	foreach(auto fileinfo,dir.entryInfoList(QDir::Files)) {
		total_size += fileinfo.size();
	}

	foreach(auto subDir,dir.entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
		total_size += GetFileSize_B(path+QDir::separator() + subDir);
	}
	return total_size;
}
qint64 Utility::GetFileSize_KB(QString path)
{
	return GetFileSize_B(path)/1024;
}
void Utility::Sleep(unsigned long _Duration)
{
	//qDebug("enter  %d ms",_Duration);
	_sleep(_Duration);
	//qDebug("leaver");
}
void Utility::DumpHex(const uint8_t* input, int32_t len,const char *tag)
{
	int32_t bank = 8;
	int offset = 0;
	qDebug("%s total len %d :", tag, len);
	for (; offset < len;) {
		QString tmp;
		int index = 0;
		for (; (index < bank) && ((index + offset) < len);index++) {
			tmp.append(QString::asprintf(" %02x ", input[index + offset]));
		}
		qInfo("%s", tmp.toStdString().c_str());
		offset += index;
	}
}
int32_t Utility::NewFile(QString value,bool ishidden)
{
	int ret=0;
	QFile file(value);
	if (file.exists())goto ERROR_OUT;
	{
		int lastslah = value.lastIndexOf("/");
		QString name = value.right(value.length() - lastslah - 1);
		QString filePath = value.left(lastslah); //conf
		QDir* dir = new QDir();
		if (!dir->exists(filePath)) {
			dir->mkpath(filePath);
			if(ishidden)SetFileAttributes((LPCWSTR)filePath.unicode(), FILE_ATTRIBUTE_HIDDEN);
		}
		if (file.open(QIODevice::WriteOnly)) {
			ret = 0;
		}
		else {
			ret = -ERROR_FILE_CORRUPT;
		}
		delete dir;
		file.close();
	}

ERROR_OUT:
	return ret;
}

int32_t Utility::NewDir(QString value)
{
	bool ret = true;
	QDir* dir = new QDir();
	if (!dir->exists(value)) {
		ret = dir->mkpath(value);
	}
	delete dir;
	return ret ? 0 : -ERROR_DIRECTORY;
}
const QString Utility::buildDateTime(QString prex)
{
	static const QDate buildDate = QLocale(QLocale::English).toDate(QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
	static const QTime buildTime = QTime::fromString(__TIME__, "hh:mm:ss");
	return QString("%1: %2 %3").arg(prex).arg(buildDate.toString("yyyy.MM.dd"), buildTime.toString());
}
#if 1
void __cdecl Utility::OutputDebugPrintf(const char* format, ...)
{
	char buf[4096], * p = buf;
	va_list args;
	va_start(args, format);
	p += _vsnprintf(p, sizeof buf - 1, format, args);
	va_end(args);
	{
		char nbuf[4096];
		snprintf(nbuf,sizeof(nbuf),"%s %s\r\n","MIYADebug", buf);
		OutputDebugStringA(nbuf);
	}
}
#endif
QString Utility::ShortIntToBrinaryString(int32_t input, int32_t len)
{
	QString input_str(input);
	input_str.setNum(input, 2);
	QString output = "";
	if ( input_str.size()>= len) {
		output = input_str.right(len);
	}
	else {
		output.fill('0', len - input_str.size());
		output.append(input_str);
	}
	return output;
}

QStringList Utility::PnPDeviceFilter(QString filter_input, QStringList res_input)
{
	//USBTestAndMeasurementDevice/4499341
	QStringList output;
	output.clear();
	QString str_class = "Ports";
	QString filter = filter_input;
	QStringList outputresult = {};
	if (filter_input.size() == 0) {
		goto ERROR_OUT;
	}
	if (filter_input.split("/").size()>1) {
		str_class = filter_input.split("/").at(0);
		filter = filter_input.split("/").at(1);
	}
	// str_class 是usb id的不需要去read com port
	if (str_class == "USBTestAndMeasurementDevice") {
		outputresult.append(filter);
		goto START_FILTER;
	}
	{
		QString script_path = "powershell.exe";
		QStringList commands;
		commands.append("-Command");
		commands.append(QString("Get-PnpDevice -Status \"OK\" -Class \"%1\"  |Select-Object FriendlyName,InstanceId | ConvertTo-Json").arg(str_class));
		QProcess* p = new QProcess();
		p->setReadChannel(QProcess::StandardOutput);
		QObject::connect(p, &QProcess::readyReadStandardOutput, [p, filter, &outputresult]() {
			QString output(p->readAllStandardOutput());
			//qDebug() << "output" << output;
			QJsonParseError jsonError;
			QJsonDocument jsondoc = QJsonDocument::fromJson(output.toLocal8Bit(), &jsonError);
			if (jsondoc.isNull()) {
				qCritical("tojsondoc fail");
			}
			else {
				if (jsondoc.isArray())
				{
					QJsonArray jsonArray = jsondoc.array();
					int nSize = jsonArray.size();
					for (int i = 0; i < nSize; i++) {
						auto val = jsonArray.at(i);
						if (val.isObject()) {
							auto obj = val.toObject();
							if (obj.contains("FriendlyName") && obj.contains("InstanceId")) {
								QJsonValue value = obj.value("InstanceId");
								if (value.toString().contains(filter)) {
									//qDebug() << value;
									auto result = obj.value("FriendlyName").toString();
									QRegularExpression re("([^(]*)[(](?<comport>\\w+)[)]");//done
									QRegularExpressionMatch match = re.match(result);
									if (match.hasMatch()) {
										//qDebug("filter output %s",match.captured("comport").toStdString().c_str());
										outputresult.append(match.captured("comport"));
									}
									else {
										qCritical("unmatch");
									}
								}

							}
						}
					}
				}

			}
			});
	
		p->start(script_path, commands);

		p->waitForFinished();
	}
START_FILTER:
	if (outputresult.size()==0) {
		goto ERROR_OUT;
	}
	//从 res_input 中过滤出outputresult包涵的内容
	{
		//1.usb 2.port
		for each (auto hwdes in outputresult)
		{
			QString target = "";
			if (hwdes.contains(QRegExp("^\\d+$"))) {
				target = hwdes;
			}
			else {
				if (hwdes.startsWith("COM")) {
					target = QString("ASRL%1::INSTR").arg(hwdes.right(hwdes.size()-strlen("COM")));
				}
				else {
					qCritical("can't handle hwdes %s", hwdes.toStdString().c_str());
					output.clear();
					goto ERROR_OUT;
				}
			}
			for each (auto visares in res_input)
			{
				if (visares.contains(target)) {
					output.append(visares);
					break;
				}
			}
		}
	}
ERROR_OUT:
	return output;
}
