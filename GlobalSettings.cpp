#include "GlobalSettings.h"
#include <QFile>
#include <winerror.h>
GlobalSettingsPtr GlobalSettings::__instance = nullptr;
int32_t GlobalSettings::SetLogDirLocation(QString value)
{
	if (QDir(value).exists()||isUserRoot()) {
		msettingsparam.global_log_dir = value;
		//msettingsparam.save();
	}
	return 0;
}
GlobalSettings::~GlobalSettings()
{
	Utility::OutputDebugPrintf("~GlobalSettings");
}
int32_t GlobalSettings::SetTestCaseResultDirLocation(QString value)
{
	if (QDir(value).exists() || isUserRoot()) {
		msettingsparam.global_testcaseResult_dir=value;
		//msettingsparam.save();
	}

	return 0;
}
int32_t GlobalSettings::LoadDefaultSettings()
{
		int ret = msettingsparam.load();
		if (ret == 0) {
			msettingparamload = true;
		}
		DumpSettingParam();
		return ret;
}
int32_t GlobalSettings::SaveSettings()
{

	int ret = msettingsparam.save();
	//qDebug("ret %d ", ret);
	DumpSettingParam();
	return ret;
}
void GlobalSettings::DumpSettingParam(bool to_log)
{
	if (to_log)qInfo("SettingParam %s", msettingsparam.to_string().toStdString().c_str());
	else qDebug("SettingParam %s", msettingsparam.to_string().toStdString().c_str());
}
void GlobalSettings::SetCurrentOp(QString val)
{
		mCurrentOp = val;
}
QString GlobalSettings::GetCurrentOp()
{
	if (isUserRoot()) {
		return "Root";
	}
	else {
		return mCurrentOp;
	}
}
bool GlobalSettings::IsSettingParamLoad()
{
	return msettingparamload;
}
QString GlobalSettings::Getbgsizemsg()
{
	return bgsizemsg;
}
GlobalSettings::GlobalSettings()
{
	LoadDefaultSettings();
	//开始统计日志，数据占用空间
	{
		ThreadworkControllerPtr ptr(new ThreadworkController(std::bind(&GlobalSettings::mbgworkloop, this)));
		mautorunthread = ptr;
	}
}
QString GlobalSettings::GetLogDirLocation() {
	return msettingsparam.GetVaildLogDir();
}
QString GlobalSettings::GetTestCaseResultDirLocation()
{
	return msettingsparam.GetVaildResultDir();
};
;
GlobalSettingsPtr GlobalSettings::get_instance()
{
	if (!__instance)
	{
		GlobalSettingsPtr ptr(new GlobalSettings);
		__instance = ptr;
	}
	return __instance;
}

QString GlobalSettings::GetVersion()
{
	return QString("%1 [%2]").arg("GlobalSettings").arg(GSVERSION);
}

int32_t GlobalSettings::SetUserRoot(QString usrname, QString passwd)
{
	if (msettingsparam.is_root)return 0;
	if (usrname=="root" && passwd=="dogoodnotevil") {
		msettingsparam.is_root = true;
		return 0;
	}
	else {
		qInfo("Fail to root [%s]:[%s]", usrname.toStdString().c_str(), passwd.toStdString().c_str());
	}
	return -1;
}

bool GlobalSettings::isUserRoot()
{
	return msettingsparam.is_root;
};
int32_t GlobalSettings::mbgworkloop()
{
	QString logdir = GetLogDirLocation();
	QString resultdir = GetTestCaseResultDirLocation();
	bgsizemsg = "";
	{
		auto starttime = QDateTime::currentDateTime().toTime_t();
		qint64 resultsize = Utility::GetFileSize_KB(resultdir);
		qint64 logsize = Utility::GetFileSize_KB(logdir);
		qInfo("Result dir [%s] size %lld KB,Log dir [%s] size %lld KB,cost %lld S",
			resultdir.toStdString().c_str(),
			resultsize,
			logdir.toStdString().c_str(),
			logsize,
			QDateTime::currentDateTime().toTime_t()-starttime);
		if (logsize > mlogmaxsize_K) {
			bgsizemsg.append(QStringLiteral("日志目录 %1 达到 %2 KB,请删除过时的不需要的文件\n").
			arg(logdir.toStdString().c_str()).
			arg(logsize));
		}
		if (resultsize > mresultmaxsize_K) {
			bgsizemsg.append(QStringLiteral("测试结果数据 %1 达到 %2 KB,请删除过时的不需要的文件\n").
				arg(resultdir.toStdString().c_str()).
				arg(resultsize));
		}
	}
	return 0;
}
int32_t GlobalSettings::SettingsParam::load(QString path)
{
	int ret = 0;
	QString settingspath = path;

	if (settingspath.size() == 0) { 
		settingspath = global_settings_path; 
		if (!QFile(settingspath).exists()) {
			//启用默认设置
			{
				ret = save(settingspath);
			}
			goto ERROR_OUT;
		}
	}
	{
	QFile settingfile(settingspath);
	if (settingspath.size()==0 || !settingfile.exists()) {
		qCritical("settingfile %s invaild", settingfile.fileName().toStdString().c_str());
	}
	{
		QSettings* settings;
		settings = new QSettings(settingspath, QSettings::IniFormat);
		bool nead_save = false;
		if (settings == nullptr) {
			ret = -ERROR_FILE_CORRUPT;
			goto ERROR_OUT;
		}
		settings->setIniCodec("UTF-8");
		{
			QStringList childs = settings->childKeys();
			foreach(auto key, childs) {
				auto value = settings->value(key).toString();
				//check path
				if (key==SETTING_FILE) {
					if(QFileInfo(value).exists()){
					global_settings_path = value;
					}
					else {
						qCritical("value %s inexistence default to %s",value.toStdString().c_str(),
							global_settings_path.toStdString().c_str());
						nead_save = true;
					}
				}
				else if (key == LOG_DIR) {
					if(QDir(value).exists()) {
						global_log_dir = value;
					}
					else {
						qCritical("value %s inexistence default to %s", value.toStdString().c_str(),
							global_log_dir.toStdString().c_str());
						nead_save = true;
					}
				}
				else if (key == TCRESULT_DIR) {
					if (QDir(value).exists()) {
						global_testcaseResult_dir = value;
					}
					else {
						qCritical("value %s inexistence default to %s", value.toStdString().c_str(),
							global_testcaseResult_dir.toStdString().c_str());
						nead_save = true;
					}
				}
			}
		}
	ERROR_SETTINGS:
		if (nead_save)save();
		ret = 0;
		delete settings;
	
	}
	}
ERROR_OUT:

	return ret;
}
int32_t GlobalSettings::SettingsParam::save(QString path)
{
	//只有在这里新建
	int ret = 0;
	QString settingspath = path;

	if (settingspath.size() == 0) {
		settingspath = global_settings_path;
	}
	if (!QFile(settingspath).exists()) {
		ret = Utility::NewFile(settingspath,true);
		if (ret != 0 || !QFile(settingspath).exists()) {
			qCritical("settingfile %s invaild", settingspath.toStdString().c_str());
			goto ERROR_OUT;
		}
	}
	{
	auto m_psetting = new QSettings(settingspath, QSettings::IniFormat);
	if (m_psetting == nullptr) {
		ret = -ERROR_FILE_CORRUPT;
		goto ERROR_OUT;
	}
	//先确保所有参数有效
	auto checkdir_vaild = [](QString value)->int32_t {
		int ret = 0;
		ret = Utility::NewDir(value);
		return ret;
	};
	auto checkpath_vaild = [](QString value)->int32_t {
		int ret = 0;
		if (!QFile(value).exists()) {
			ret = Utility::NewFile(value);
			if (ret != 0 || !QFile(value).exists()) {
				qCritical("settingfile %s invaild", value.toStdString().c_str());
				return -1;
			}
		}
		return ret;
	};
	ret = checkpath_vaild(global_settings_path);
	if (ret != 0 ) {
		goto ERROR_OUT;
	}
	ret = checkdir_vaild(global_log_dir);
	if (ret != 0) {
		goto ERROR_OUT;
	}
	ret = checkdir_vaild(global_testcaseResult_dir);
	if (ret != 0) {
		goto ERROR_OUT;
	}
	//qDebug(" ");
	m_psetting->clear();
	m_psetting->setValue(SETTING_FILE, QFileInfo(global_settings_path).absoluteFilePath());
	m_psetting->setValue(LOG_DIR, QDir(global_log_dir).absolutePath());
	m_psetting->setValue(TCRESULT_DIR, QDir(global_testcaseResult_dir).absolutePath());
	delete m_psetting;
	}
ERROR_OUT:

	return ret;
}

QString GlobalSettings::SettingsParam::GetVaildLogDir()
{
	if (!QDir(global_log_dir).exists()) {
		auto tmp = new SettingsParam();
		global_log_dir = tmp->global_log_dir;
		delete tmp;
		save();
	}
	return global_log_dir;
}

QString GlobalSettings::SettingsParam::GetVaildResultDir()
{
	if (!QDir(global_testcaseResult_dir).exists()) {
		auto tmp = new SettingsParam();
		global_testcaseResult_dir = tmp->global_testcaseResult_dir;
		delete tmp;
		save();
	}
	return global_testcaseResult_dir;
}
