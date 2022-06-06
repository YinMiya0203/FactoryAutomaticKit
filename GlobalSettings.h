#pragma once
#include <QString>
#include <memory>
#include "Utility.h"
#include "MessagePacket.h"

#define GSVERSION_M	"0"
#define GSVERSION_S	"0"
#define GSVERSION_P	"1"
#define GSVERSION GSVERSION_M "." GSVERSION_S "." GSVERSION_P
#define SETTING_FILE "SETTING_FILE"
#define LOG_DIR	"LOG_DIR"
#define TCRESULT_DIR	"TCRESULT_DIR"
class GlobalSettings;
#define GLOBALSETTINGSINSTANCE GlobalSettings::get_instance() 
typedef std::shared_ptr<GlobalSettings> GlobalSettingsPtr;
class GlobalSettings
{
public:
	class SettingsParam {
		friend GlobalSettings;
	private:
		int32_t load(QString path = "");
		int32_t save(QString path = "");
		QString GetVaildLogDir();
		QString GetVaildResultDir();
	private:
		QString global_settings_path= "./.settings/defaultSettings.ini";
		QString global_log_dir="./LOG";
		QString global_testcaseResult_dir="./Result";
		bool is_root=false; //²»±£´æ
		QString to_string() {
			return QString("%1:[%2] %3:[%4] %5:[%6] ").arg(SETTING_FILE).arg(global_settings_path)	\
				.arg(LOG_DIR).arg(global_log_dir) \
				.arg(TCRESULT_DIR).arg(global_testcaseResult_dir);
		}
	};
	static GlobalSettingsPtr get_instance();
	static QString GetVersion();
	int32_t SetUserRoot(QString usrname,QString passwd);
	bool isUserRoot();
	int32_t SetLogDirLocation(QString value);
	~GlobalSettings();
	int32_t SetTestCaseResultDirLocation(QString value);
	int32_t LoadDefaultSettings();
	int32_t SaveSettings();
	void DumpSettingParam();

	QString GetLogDirLocation();
	QString GetTestCaseResultDirLocation();

	void SetCurrentOp(QString val);
	QString GetCurrentOp();
	bool IsSettingParamLoad();
private:
	GlobalSettings();
	static GlobalSettingsPtr __instance;
	SettingsParam msettingsparam;
	QString mCurrentOp;
	bool msettingparamload = false;
};

