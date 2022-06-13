#include "Utility.h"
#include <winerror.h>
QString Utility::GetWinErrorText(int32_t errcode) {
	int len = 256;
	static CHAR* pmsg = (CHAR*)malloc(len * sizeof(CHAR));
	if (errcode < 0)errcode = -errcode;
	FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errcode,
		//»ñµÃÈ±Ê¡ÓïÑÔ
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		pmsg,
		len,
		NULL);
	//printf("errcode %x ´íÎóÃèÊö£º%s", errcode, pmsg);
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