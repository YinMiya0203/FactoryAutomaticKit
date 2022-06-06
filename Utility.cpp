#include "Utility.h"
#include <winerror.h>
int32_t Utility::NewFile(QString value)
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