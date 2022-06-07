#include "CaseItemBase.h"
#include "TestCase.h"
#include <QRegularExpression> 
#include <QMessageBox>
#include <QDateTime>
CaseItemBasePtrContain CaseItemBase::mstaticcaseitemptrcontain = {};
CaseItemBasePtr CaseItemBase::get_instance(QSettings* settings,int offset)
{
	if (offset< mstaticcaseitemptrcontain.size()) {
		return mstaticcaseitemptrcontain[offset];
	}
	CaseItemContainer mcitems;
	if (!checkparam(settings, mcitems)) {
		return nullptr;
	}
	
	CaseItemBasePtr ptr(new CaseItemBase(mcitems));
	ptr->moffset_inlist = offset;
	mstaticcaseitemptrcontain.push_back(ptr);
	return ptr;
}

CaseItemBase::~CaseItemBase()
{
	OutputDebugStringA("~CaseItemBase\r\n");
}

CaseItemBase::CaseItemBase(CaseItemContainer mcitems):mcaseitems(mcitems)
{
	if (GlobalConfig_debugtestcaseini)qDebug("CaseItemBase items %d", mcaseitems.size());
	mtypecontainer.clear();
	Getitemtype(0);
	RegisterHandles(&mcaseitemhandles);
	RegisterShows(&mcaseItemshows);
}
caseitem_type CaseItemBase::Getitemtype(int mstep)
{

	if (mstep >= mcaseitems.size()) {
		goto ERR_OUT;
	}
	if (mtypecontainer.size() == 0) {
		foreach(auto items, mcaseitems)
		{
			caseitem_type type = caseitem_type::unknown;
			if (items.first == caseitem_class::Passcondition ||
				items.first == caseitem_class::ManualConfirm) {
				type = caseitem_type::ResultData;
			}
			else if (items.first != caseitem_class::Unknown &&
				items.first != caseitem_class::Comment/* &&
				mcaseitems[step].first != caseitem_class::Ignore*/) {
				type = caseitem_type::Contition;
			}
			//qDebug("mtypecontainer %d class %d type %d", mtypecontainer.size(), items.first, type);
			mtypecontainer.push_back(type);
		}
#if 0
		if (mcaseitems[mstep].first == caseitem_class::Passcondition ||
			mcaseitems[mstep].first == caseitem_class::ManualConfirm) {
			type = caseitem_type::ResultData;
		}
		else if (mcaseitems[mstep].first != caseitem_class::Unknown &&
			mcaseitems[mstep].first != caseitem_class::Comment/* &&
			mcaseitems[step].first != caseitem_class::Ignore*/) {
			type = caseitem_type::Contition;
		}
#endif
	}

	return mtypecontainer[mstep];
	
ERR_OUT:
	return caseitem_type::unknown;
}
int32_t CaseItemBase::GetConditioncnt()
{
	int cnt = 0;
	if (mconditioncnt >= 0)return mconditioncnt;
	foreach (auto items, mtypecontainer)
	{
		if (items == caseitem_type::Contition) {
			cnt++;
		}
	}
	mconditioncnt = cnt;
	return cnt;
}
QString CaseItemBase::GetConditionShowTotal()
{
	QString raw = "";
	int cnt = 0;
	foreach(auto items, mtypecontainer)
	{
		if (items == caseitem_type::Contition) {
			QString tmp = "";
			Getitemshow(cnt,tmp);
			if(cnt!=0)raw.append(";");
			raw.append(tmp);
		}
		cnt++;
	}
	return raw;
}
int32_t CaseItemBase::GetResultitemcnt()
{
	int cnt = 0;
	if (mresultitemcnt >= 0)return mresultitemcnt;
	foreach(auto items, mtypecontainer)
	{
		if (items == caseitem_type::ResultData) {
			cnt++;
		}
	}
	mresultitemcnt = cnt;
	return cnt;
}
int32_t CaseItemBase::Getitemhandle(int mstep)
{
	int ret = 0;
	caseitembase_handle handle = nullptr;
	if (mstep >= mcaseitems.size()) {
		ret = -ERROR_NOT_SUPPORTED;
		goto ERR_OUT;
	}
	//Commenthandle
#define case_handle(xxx) case caseitem_class::##xxx: \
	handle = mcaseitemhandles.##xxx##handle; break;

	switch (mcaseitems[mstep].first) {
		case_handle(Comment)
			case_handle(Delayms)
			case_handle(Ignore)
			case_handle(ManualConfirm)
			case_handle(ManualNotice)
			case_handle(Passcondition)
			case_handle(Precondition)
	default:
		break;
	}
	if (handle == nullptr) {
		ret = DefaultCaseItemBaseHandle(mcaseitems[mstep].second, mstep);
	}
	else {
		ret = handle(mcaseitems[mstep].second, mstep);
	}
ERR_OUT:
	{
	//toview color 先刷文字再刷状态
		auto msg = new MessageTVCaseItemWidgetStatus;
		msg->is_success = (ret == 0);
		msg->sector = moffset_inlist;
		msg->seek = mstep;
		MessageTVBasePtr mptr(msg);
		emit notifytoView(int(msg->GetCmd()), mptr);
	}
	return ret;
}

int32_t CaseItemBase::Getitemshow(int mstep, ShowString & output)
{
	int ret = 0;
	caseitembase_show callback = nullptr;
	if(mstep >=mcaseitems.size()) {
		ret = -ERROR_NOT_SUPPORTED;
		goto ERR_OUT;
	}
#define case_show(xxx) case caseitem_class::##xxx: \
	callback = mcaseItemshows.##xxx##show; break;

	switch(mcaseitems[mstep].first) {
		case_show(Comment)
		case_show(Delayms)
		case_show(Ignore)
		case_show(ManualConfirm)
		case_show(ManualNotice)
		case_show(Passcondition)
		case_show(Precondition)
		default:
			break;
	}
	if (callback!= nullptr) {
		callback(mcaseitems[mstep].second, output);
	}
	else {
		DefaultCaseItemBaseShow(mcaseitems[mstep].second, output);
	}
ERR_OUT:
	return ret;
}
bool CaseItemBase::isCaseIgnore()
{
	return misignore;
}
int32_t CaseItemBase::Process(int mstep)
{
	int ret = 0;
	do {
		if (mstep > mcaseitems.size()) {
			ret = -ERROR_INVALID_DATA;
			break;
		}
		if (mstep == mcaseitems.size()) {
			ret = error_itemseekend;
			break;
		}
		if (isCaseIgnore()) {
			ret = error_casesectorend;
			break;
		}
		ret = Getitemhandle(mstep);
		
	} while (0);
	return ret;
}


int32_t CaseItemBase::Getitemtypeshow(caseitem_class type, ShowString & output)
{
	int ret = 0;
	output.clear();
	//only comment support
	if (type != caseitem_class::Comment) {
		ret = -ERROR_NOT_SUPPORTED;
		goto ERR_OUT;
	}
	foreach(auto item, mcaseitems) {
		if (item.first == caseitem_class::Comment) {
			if(mcaseItemshows.Commentshow!=nullptr)mcaseItemshows.Commentshow(item.second, output);
			break;
		}
	}
ERR_OUT:
	return ret;
}
bool CaseItemBase::checkparam(QSettings* settings, CaseItemContainer & itemlist)
{
	bool ret = true;
	itemlist.clear();
	foreach(auto key, settings->allKeys()) {

		auto value = settings->value(key).toString().trimmed();
		if (value.startsWith("commontag", Qt::CaseInsensitive)) {
			value = TestcaseBase::get_instance()->GetCommontag(value);
		}
		if (GlobalConfig_debugtestcaseini)qDebug("key %s === %s", key.toStdString().c_str(), value.toStdString().c_str());
		bool checked = true;
		caseitem_class type = getCaseItemclass(key);
		if (type==caseitem_class::Unknown) {
			checked = false; goto ERR_OUT;
		}
		if (type== caseitem_class::Comment || 
			type == caseitem_class::ManualConfirm ||
			type == caseitem_class::ManualNotice ||
			type == caseitem_class::Delayms||
			type == caseitem_class::Ignore) {
			if (value.size() > 0) { 
				CaseItemPair mpair = std::make_pair(type, value.toStdString());
				itemlist.push_back(mpair);
				continue; 
			}else { checked = false; goto ERR_OUT; }
		}
		if (type == caseitem_class::Precondition ||
			type == caseitem_class::Passcondition){
			//这里只确认网络标号存在与device了
			int index = 0;
			std::string networklabel = value.split('/').front().toStdString();
			if(TestcaseBase::get_instance()->GetDeviceId(networklabel, index)!=0)
			{
				qCritical("network %s no find", networklabel.c_str());
				checked = false; goto ERR_OUT;
			}
			else {
				CaseItemPair mpair = std::make_pair(type, value.toStdString());
				itemlist.push_back(mpair);
				continue;
			}
		}
	ERR_OUT:
		if (!checked) { 
			qCritical("case checkingParam fail");
			ret = false; break; 

		}
	}
	if (GlobalConfig_debugtestcaseini)qDebug("ret %d", ret);
	return ret;
}
caseitem_class CaseItemBase::getCaseItemclass(QString input)
{
	if (caseitem_class_map.size() <= 0) {
		caseitem_class_map[QString("comment")] = caseitem_class::Comment;
		caseitem_class_map[QString("ignore")] = caseitem_class::Ignore;
		caseitem_class_map[QString("delayms")] = caseitem_class::Delayms;
		caseitem_class_map[QString("manualnotice")] = caseitem_class::ManualNotice;
		caseitem_class_map[QString("manualconfirm")] = caseitem_class::ManualConfirm;
		caseitem_class_map[QString("precondition")] = caseitem_class::Precondition;
		caseitem_class_map[QString("passcondition")] = caseitem_class::Passcondition;
	}
	QString input_raw = input.toLower().split('_').back();
	caseitem_class type = caseitem_class::Unknown;
	if (caseitem_class_map.count(input_raw) != 0) {
		type = caseitem_class_map[input_raw];
	}

	return type;
}

int32_t CaseItemBase::DefaultCaseItemBaseHandle(std::string input, int mstep)
{
	int ret = 0;
	if(GlobalConfig_debugCaseItemBase)qDebug("input %s",input.c_str());
	return ret;
}

int32_t CaseItemBase::DefaultCaseItemBaseShow(std::string input, ShowString& output)
{
	int ret = 0;
	output = ShowString(input.c_str());
	return ret;
}

int32_t CaseItemBase::CaseItemDelaymsShow(std::string input, ShowString& output)
{
	int ret = 0;
	output = ShowString("%1 %2 ms").arg(QStringLiteral("延时")).arg(atoi(input.c_str()));
	return ret;
}
int32_t CaseItemBase::CaseItemManualNoticeShow(std::string input, ShowString& output)
{
	if (std::find(input.begin(),input.end(),'/')!=input.end()) {
		translation_slash_smart(input, output,caseitem_class::ManualNotice);
		return output.size() > 0 ? 0 : -ERROR_INVALID_PARAMETER;
	}else
	return DefaultCaseItemBaseShow(input,output);
}
int32_t CaseItemBase::CaseItemManualConfirmShow(std::string input, ShowString& output)
{
	if (std::find(input.begin(), input.end(), '/') != input.end()) {
		translation_slash_smart(input, output, caseitem_class::ManualConfirm);
		return output.size() > 0 ? 0 : -ERROR_INVALID_PARAMETER;
	}
	else
		return DefaultCaseItemBaseShow(input, output);
}

IniInfoBasePtr CaseItemBase::translation_slash_smart(std::string input, QString& output, caseitem_class type)
{
	QString input_str = QString(input.c_str());
	output.clear();
	IniInfoBasePtr ptr = nullptr;
	//int slash_cnt = input_str.count('/');
#define PURE_FLOAT_RE "-?\\d{1,}(\\.[0-9]+)?"
#define PURE_UINT_RE "\\d+"
	if (type == caseitem_class::Precondition) {
		ptr = PreconditionWithNetworkId(input_str, output);
		goto ERROR_OUT;
	}else if((type == caseitem_class::Passcondition )||
		(type == caseitem_class::ManualConfirm)){
		if (type== caseitem_class::ManualConfirm && input_str.contains("Resource:",Qt::CaseInsensitive)) {
			ptr = ManualConfirmWithRes(input_str, output);
			goto ERROR_OUT;
		}
		else {
			ptr = PassconditionWithNetworkId(input_str,output);
			goto ERROR_OUT;
		}
	}
	else if (type == caseitem_class::ManualNotice) {
		ptr = ManualConfirmWithRes(input_str, output);
		goto ERROR_OUT;
	}
	if(output.size()==0) {
		auto out = input_str.split('/');
		foreach(auto v, out) {
			output.append(v);
			output.append(' ');
		}
	}
ERROR_OUT:
	return ptr;
}
IniInfoBasePtr CaseItemBase::PreconditionWithNetworkId(QString input_str, QString& output)
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
		qCritical("input %s match fail",input_str.toStdString().c_str());
	}
ERROR_OUT:
	return ptr;
}
IniInfoBasePtr CaseItemBase::ManualConfirmWithRes(QString input_str, QString& output)
{
	IniInfoBasePtr ptr = nullptr;
	auto info = new ManualViewInfoBase;
	auto raw_list = input_str.split('/');
	foreach(auto str, raw_list) {
		if (str.contains("Resource:", Qt::CaseInsensitive)) {
			info->resource.push_back(str.right(str.size()-QString("Resource:").size()));
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
IniInfoBasePtr CaseItemBase::PassconditionWithNetworkId(QString input_str, QString& output)
{
	IniInfoBasePtr ptr = nullptr;
	QRegularExpression re("(?<networklabel>\\w+)/(?<mode>\\w+)/[(](?<rangmin>" PURE_FLOAT_RE ")(?<rangminunit>\\w+)/(?<rangmax>" PURE_FLOAT_RE ")(?<rangmaxunit>\\w+)[)]/(?<duration>" PURE_UINT_RE ")(?<durationunit>\\w+)");
	QRegularExpressionMatch match = re.match(input_str);
	if (match.hasMatch()) {
		auto info = new NetworkLabelPassconditionBase;
		info->networklabel = match.captured("networklabel");
		info->mode = match.captured("mode");
		{
			int uintmin = 1000, uintmax = 1000;

			QString rangminunit = match.captured("rangminunit");
			QString rangmaxunit = match.captured("rangmaxunit");
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
			info->rangmin_m = match.captured("rangmin").toDouble() * uintmin;
			info->rangmax_m = match.captured("rangmax").toDouble() * uintmax;
			info->unit = unit;
			//qDebug("rangmin %s %lf ", match.captured("rangmin").toStdString().c_str(), info->rangmin_m);
			//qDebug("rangmax %s %lf unit[%s]", match.captured("rangmax").toStdString().c_str(), info->rangmax_m, unit.toStdString().c_str());

		}
		{
			int duration_ms = match.captured("durationunit").toUpper().compare("MS") == 0 ? 1 : 1000;
			duration_ms *= match.captured("duration").toInt();
			info->duration_ms = duration_ms;
			//qDebug("duration_ms %d [%s][%s]", duration_ms, match.captured("duration").toStdString().c_str(),match.captured("durationunit").toStdString().c_str());

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
int32_t CaseItemBase::CaseItemPreconditionShow(std::string input, ShowString& output)
{
	//006_precondition=Vdc/12V/0	;;网络标号/数值/持续时间(0在下次修改前保持不动)
	translation_slash_smart(input, output, caseitem_class::Precondition);
	return output.size()>0?0:-ERROR_INVALID_PARAMETER;
}
int32_t CaseItemBase::CaseItemPassconditionShow(std::string input, ShowString& output)
{
	//007_passcondition = ALi / immediate / (700ma / 900ma) / 5s;; 网络标号 / 类型(平均，峰值，) / (rang1 / rang2) / 时长
	translation_slash_smart(input, output, caseitem_class::Passcondition);
	return output.size() > 0 ? 0 : -ERROR_INVALID_PARAMETER;
}
int32_t CaseItemBase::CaseItemIgnoreShow(std::string input, ShowString& output)
{
	int ret = 0;
	if( strcmpi(input.c_str(),"TRUE") == 0) {
		misignore = true;
		output = ShowString("%1").arg(QStringLiteral("忽略此测试项"));
	}
	else {
		output = ShowString("%1").arg(QStringLiteral("正常项"));
	}

	return ret;
}
int32_t CaseItemBase::CaseItemCommentHandle(std::string input, int mstep)
{
	return 0;
}
int32_t CaseItemBase::CaseItemIgnoreHandle(std::string input, int mstep)
{
	return 0;
}
int32_t CaseItemBase::CaseItemDelaymsHandle(std::string input, int mstep)
{
	QString input_value = QString(input.c_str());
	bool need_ui = false;
	bool had_res = false;
	int32_t delayms = 0;
	QString msg="";
	QString res="";
	int ret = 0;
	if (input_value.count('/') > 0) {
		QStringList param = QString(input.c_str()).split('/');
		foreach(auto item, param) {
			if (item.contains(QRegExp("^\\d+$"))) {
				delayms = item.toInt();
				if (delayms > 3000) {
					need_ui = true;
				}
			}
			else if (item.contains("Resource:",Qt::CaseInsensitive)) {
				had_res = true;
				need_ui = true;
				res = item.right(item.size()-strlen("Resource:"));
			}else{
				if (!had_res)msg = QString("%1 %2").arg(item).arg(QStringLiteral("\n点击 取消 按键将终止本次测试"));
				else {
					msg = QString("%1").arg(item);
				}
				//else msg = QStringLiteral("\n点击 取消 按键将终止本次测试");
			}
		}
	}
	else {
		delayms = QString(input.c_str()).toInt();
	}
	if (GlobalConfig_debugCaseItemBase)qDebug("delayms %d ms ui %d", delayms, need_ui);
	if (need_ui){
		//QMessageBox::warning(NULL, "hehe", "yahha", QMessageBox::Yes);//不能在背后偷偷弹啊
		auto mmsg = new MessageTVCaseCountDownDialog;
		mmsg->durationms = delayms;
		mmsg->msg = msg;
		mmsg->resource = res;
		MessageTVBasePtr ptr(mmsg);
		emit notifytoView(int(mmsg->GetCmd()), ptr);
		QMutexLocker locker(&(mmsg->mutex));
		int result = mmsg->mwait.wait(&(mmsg->mutex), delayms+1000);
		if (!mmsg->is_success) {//true dialog cancel
			if (GlobalConfig_debugCaseItemBase)qInfo("user cancel");
			ret = error_caseusrtermin;
		}
		else {
			ret = 0;
		}

	}
	else {
		Sleep(delayms);
		ret = 0;
	}
	return ret;
}
int32_t CaseItemBase::CaseItemManualNoticeHandle(std::string input, int mstep)
{
	int ret = 0;
	if (GlobalConfig_debugCaseItemBase)qDebug(" input %s", input.c_str());
	auto mmsg = new MessageTVCaseNoticeDialog;
	CaseItemManualNoticeShow(input,mmsg->msg);
	MessageTVBasePtr ptr(mmsg);
	emit notifytoView(int(mmsg->GetCmd()), ptr);
	QMutexLocker locker(&(mmsg->mutex));
	mmsg->mwait.wait(&(mmsg->mutex));
	return ret;
}
int32_t CaseItemBase::CaseItemManualConfirmHandle(std::string input, int mstep)
{
	int ret = 0;
	QString input_value = QString(input.c_str());
	bool is_needinput = false;
	bool had_res = input_value.contains("Resource:",Qt::CaseInsensitive);
	if (input_value.count('/') > 0 && !had_res) {
		is_needinput = true;
	}
	if (!is_needinput) {
		//没有输入框
		QString showinput;
		auto mmsg = new MessageTVCaseConfirmDialog;
		auto info_raw = translation_slash_smart(input, showinput, caseitem_class::ManualConfirm);
		ManualViewInfoBase* info = dynamic_cast<ManualViewInfoBase*>(info_raw.get());
		if (info != nullptr) {
			mmsg->resource = info->resource;
		}
		CaseItemManualConfirmShow(input, showinput);
		mmsg->msg = QStringLiteral("%1 %2\n%3\n%4").arg(QStringLiteral("请确认")) \
			.arg(showinput) \
			.arg(QStringLiteral("确认成功请点击 确定 按钮")) \
			.arg(QStringLiteral("无法确认或确认失败请点击 取消 按钮"));
		MessageTVBasePtr ptr(mmsg);
		emit notifytoView(int(mmsg->GetCmd()), ptr);
		QMutexLocker locker(&(mmsg->mutex));
		mmsg->mwait.wait(&(mmsg->mutex));
		if (GlobalConfig_debugCaseItemBase)qDebug(" result 0x%x",mmsg->buttonclicked);
		if (mmsg->buttonclicked == QMessageBox::StandardButton::Yes) {
			ret = 0;
		}
		else {
			ret = -ERROR_INVALID_PARAMETER;
		}		
	}
	else {
		QString showinput;
		auto info_raw = translation_slash_smart(input, showinput,caseitem_class::ManualConfirm);
		if(info_raw->GetType()== caseiteminfo_type::PassconditionWithNetworkId){
			NetworkLabelPassconditionBase* info = dynamic_cast<NetworkLabelPassconditionBase*>(info_raw.get());
			if (info==nullptr) {
				ret = -ERROR_INVALID_PARAMETER;
				goto ERR_OUT;
			}
			auto mmsg = new MessageTVCaseConfirmWithInputsDialog;
			QString unit = info->unit;
			mmsg->msg = QStringLiteral("请确认数值(%1)并且填入对应输入框 ").arg(unit);
			mmsg->prex.push_back(info->networklabel);
			mmsg->input_value.clear();
			MessageTVBasePtr ptr(mmsg);
			emit notifytoView(int(mmsg->GetCmd()), ptr);
			QMutexLocker locker(&(mmsg->mutex));
			mmsg->mwait.wait(&(mmsg->mutex));
			if (mmsg->input_value.size()!= mmsg->prex.size()) {
				ret = -ERROR_INVALID_PARAMETER;
				goto ERR_OUT;
			}
			foreach(auto value_str , mmsg->input_value) {
				auto data = value_str.toFloat();
				if (unit.toUpper().count('M') == 0) {
					data *= 1000;
				}
				if ((data>=info->rangmin_m) && (data<= info->rangmax_m)) {
					ret = 0;
				}
				else {
					ret = -ERROR_INVALID_PARAMETER;
				}
				{
					//tov
					auto tvmsg = new MessageTVCaseItemWidgetStringUpdate;
					tvmsg->sector = moffset_inlist;
					tvmsg->seek = mstep;
					tvmsg->msg = QStringLiteral("实际结果：%1 %2").arg(mmsg->input_value.first()).arg(unit);
					MessageTVBasePtr ptr(tvmsg);
					emit notifytoView(int(tvmsg->GetCmd()), ptr);
				}
			}
		}
		else {
			ret = -ERROR_INVALID_PARAMETER;
			goto ERR_OUT;
			qCritical("info_raw->GetType() %d", info_raw->GetType());
		}
	}
ERR_OUT:
	return ret;
}
int32_t CaseItemBase::CaseItemPreconditionHandle(std::string input, int mstep)
{
	//
	int ret = 0;
	QString showinput;
	auto info_raw = translation_slash_smart(input, showinput, caseitem_class::Precondition);
	NetworkLabelPreconditionBase* info = dynamic_cast<NetworkLabelPreconditionBase*>(info_raw.get());
	if (info == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERR_OUT;
	}
	int32_t dev_id;
	ret = TestcaseBase::get_instance()->GetDeviceId(info->networklabel.toStdString(), dev_id);
	if (ret != 0) {
		qCritical("networklabel %s can't find", info->networklabel.toStdString().c_str());
		ret = -ERROR_INVALID_PARAMETER;
		goto ERR_OUT;		
	}
	{

		ret = FunctionSetVoltageOut(dev_id,info);
	}

ERR_OUT:
	return ret;
}
int32_t CaseItemBase::FunctionSetVoltageOut(int32_t dev_id, NetworkLabelPreconditionBase* msg)
{
	//set voltage;check current limit;check output on
	int ret = 0;
	if (msg == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	{
		DeviceStatus_t st;
		int32_t default_currentlimit_ma = 2 * 1000;
		ret = TestcaseBase::get_instance()->Getdevicestatus(dev_id, st);
		if (ret == 0) {
			if(msg->voltage_mv!=0)default_currentlimit_ma = (st.maxWVA*1000*1000) / (msg->voltage_mv)/1000*1000;
		}
		if (GlobalConfig_debugCaseItemBase)qDebug("default_currentlimit_ma %d  mv %d maxWVA %d", default_currentlimit_ma, msg->voltage_mv, st.maxWVA);
		auto mptrcl = VisaDriverIoctrlBasePtr(new DeviceDriverSourceCurrentLimit);
		DeviceDriverSourceCurrentLimit* upper = dynamic_cast<DeviceDriverSourceCurrentLimit*>(mptrcl.get());
		upper->is_read = true;
		ret = TestcaseBase::get_instance()->devcieioctrl(dev_id, mptrcl);
		if (ret != 0) {
			qCritical("IOCTRL DeviceDriverSourceCurrentLimit read fail");
			ret = -ERROR_DEVICE_UNREACHABLE;
			goto ERROR_OUT;
		}

		if (upper->current_ma < default_currentlimit_ma) {
			upper->is_read = false;
			upper->current_ma = default_currentlimit_ma;
			ret = TestcaseBase::get_instance()->devcieioctrl(dev_id, mptrcl);
			if (ret != 0) {
				qCritical("IOCTRL DeviceDriverSourceCurrentLimit set %d fail", upper->current_ma);
				ret = -ERROR_DEVICE_UNREACHABLE;
				goto ERROR_OUT;
			}
		}
	}
	{
		auto mptrrq = VisaDriverIoctrlBasePtr(new DeviceDriverSourceVoltage);
		DeviceDriverSourceVoltage* upper = dynamic_cast<DeviceDriverSourceVoltage*>(mptrrq.get());
		upper->is_read = true;
		ret = TestcaseBase::get_instance()->devcieioctrl(dev_id, mptrrq);
		if (ret != 0) {
			qCritical("IOCTRL DeviceDriverReadQuery fail");
			ret = -ERROR_DEVICE_UNREACHABLE;
			goto ERROR_OUT;
		}
		if (upper->voltage_mv != msg->voltage_mv) {
			//set voltage
			auto mptrsv = VisaDriverIoctrlBasePtr(new DeviceDriverSourceVoltage);
			DeviceDriverSourceVoltage* upper = dynamic_cast<DeviceDriverSourceVoltage*>(mptrsv.get());
			upper->is_read = false;
			upper->voltage_mv = msg->voltage_mv;
			ret = TestcaseBase::get_instance()->devcieioctrl(dev_id, mptrsv);
			if (ret != 0) {
				qCritical("IOCTRL DeviceDriverSourceVoltage set %d fail", upper->voltage_mv);
				ret = -ERROR_DEVICE_UNREACHABLE;
				goto ERROR_OUT;
			}

		}
	}
	//set out
	{
		auto mptrsv = VisaDriverIoctrlBasePtr(new DeviceDriverOutputState);
		DeviceDriverOutputState* upper = dynamic_cast<DeviceDriverOutputState*>(mptrsv.get());
		upper->is_read = false;
		upper->onoff = true;
		ret = TestcaseBase::get_instance()->devcieioctrl(dev_id, mptrsv);
		if (ret != 0) {
			qCritical("IOCTRL DeviceDriverOutputState set %d fail", upper->onoff);
			ret = -ERROR_DEVICE_UNREACHABLE;
			goto ERROR_OUT;
		}
	}
ERROR_OUT:
	return ret;
}
int32_t CaseItemBase::FunctionQueryCurrent(int32_t dev_id, QString &output, NetworkLabelPassconditionBase* msg)
{
	int ret = 0;
	output.clear();
	if (msg == nullptr) {
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	if (msg->mode.toUpper()!="IMMEDIATE") {
		qCritical("Mode %s can't support", msg->mode.toStdString().c_str());
		ret = -ERROR_INVALID_PARAMETER;
		goto ERROR_OUT;
	}
	{
			bool had_checked = false;
			QDateTime starttime = QDateTime::currentDateTime();
			while((!had_checked) && 
				(QDateTime::currentDateTime().toTime_t()- starttime.toTime_t() <(msg->duration_ms/1000)))
			{
				auto mptrrq = VisaDriverIoctrlBasePtr(new DeviceDriverReadQuery);
				DeviceDriverReadQuery* upper = dynamic_cast<DeviceDriverReadQuery*>(mptrrq.get());
				if (msg->networklabel.front() == 'V') {
					upper->mMeasfunc = DeviceDriverReadQuery::QueryMeasFunc::MeasDCV;
				}else if (msg->networklabel.front()=='I'|| msg->networklabel.front() == 'A') {
					upper->mMeasfunc = DeviceDriverReadQuery::QueryMeasFunc::MeasDCI;
				}
				ret = TestcaseBase::get_instance()->devcieioctrl(dev_id, mptrrq);
				if (ret != 0) {
					qCritical("IOCTRL DeviceDriverReadQuery fail");
					ret = -ERROR_DEVICE_UNREACHABLE;
					goto ERROR_OUT;
				}
				double limit[2] = { msg->rangmin_m,msg->rangmax_m };
				if (msg->unit.toUpper().count('M') == 0) {
					limit[0] *= 1000;
					limit[1] *= 1000;
				}
				{
				
					double taget = upper->current_ma;
					if (upper->mMeasfunc == DeviceDriverReadQuery::QueryMeasFunc::MeasDCV) {
						taget = upper->voltage_mv;
					}
					//qDebug("value %lf rang[%lf/%lf] unit [%s]", upper->current_ma,limit[0],limit[1], msg->unit.toStdString().c_str());
					if (taget <= limit[1] && taget >=limit[0]) {
						ret = 0;
						had_checked = true;
					}
					else {
						qCritical("value %lf out of rang [%lf/%lf]", taget, limit[0], limit[1]);
						ret = -ERROR_DATA_NOT_ACCEPTED;
						if (msg->duration_ms > 0){
							_sleep(1 * 1000);
							if (GlobalConfig_debugCaseItemBase)qDebug("time: %s",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
						}
					}
					output = QStringLiteral("实际数值:%1 %2").arg(taget).arg(msg->unit);
				}
			}
	}
ERROR_OUT:
	return ret;
}
int32_t CaseItemBase::CaseItemPassconditionHandle(std::string input, int mstep)
{
int ret = 0;
	QString showinput;
	QString result;
	auto info_raw = translation_slash_smart(input, showinput, caseitem_class::Passcondition);
	NetworkLabelPassconditionBase* info = dynamic_cast<NetworkLabelPassconditionBase*>(info_raw.get());
	int32_t dev_id;
	ret = TestcaseBase::get_instance()->GetDeviceId(info->networklabel.toStdString(), dev_id);
	if (ret != 0) {
		qCritical("networklabel %s can't find", info->networklabel.toStdString().c_str());
		ret = -ERROR_INVALID_PARAMETER;
		goto ERR_OUT;
	}

	ret = FunctionQueryCurrent(dev_id, result, info);
	{
		//toview color
		auto msg = new MessageTVCaseItemWidgetStringUpdate;
		msg->msg = result;
		msg->sector = moffset_inlist;
		msg->seek = mstep;
		MessageTVBasePtr mptr(msg);
		emit notifytoView(int(msg->GetCmd()), mptr);
	}
ERR_OUT:
	return ret;
}
void CaseItemBase::RegisterHandles(PCaseItemHandles handles)
{
#define bindhandle(xxx) handles->##xxx##handle = std::bind(&CaseItemBase::CaseItem##xxx##Handle, this, std::placeholders::_1,std::placeholders::_2);
	bindhandle(Comment)
	bindhandle(Delayms)
	bindhandle(Ignore)
	bindhandle(ManualConfirm)
	bindhandle(ManualNotice)
	bindhandle(Passcondition)
	bindhandle(Precondition)
}

void CaseItemBase::RegisterShows(PCaseItemShows shows)
{
#define bindshow(xxx) 	shows->##xxx##show = std::bind(&CaseItemBase::CaseItem##xxx##Show, this, std::placeholders::_1, std::placeholders::_2);

	shows->Commentshow = std::bind(&CaseItemBase::DefaultCaseItemBaseShow, this, std::placeholders::_1, std::placeholders::_2);
	bindshow(Delayms)
	bindshow(Ignore)
	bindshow(ManualConfirm)
	bindshow(ManualNotice)
	bindshow(Passcondition)
	bindshow(Precondition)
}

