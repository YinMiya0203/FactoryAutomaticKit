#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <QSettings>
#include "MessagePacket.h"
#define CIVERSION_M	"0"
#define CIVERSION_S	"1"
#define CIVERSION_P	"0"
#define CIVERSION CIVERSION_M "." CIVERSION_S "." CIVERSION_P
enum class caseitem_class {
	Unknown,
	Comment,
	Ignore,
	Delayms,
	ManualNotice,
	ManualConfirm,	//5
	Precondition,
	Passcondition,
};
enum class caseitem_type {
	unknown,
	Contition,
	ResultData,
};
enum class caseiteminfo_type {
	Unknown,
	PassconditionWithNetworkId,
	PreconditionWithNetworkId,
	ManualConfirmWithNetworkId,
	ManualConfirmWithRes,

};
using caseitem_typecontainer = QVector<caseitem_type>;
class IniInfoBase {
private:
	caseiteminfo_type type = caseiteminfo_type::Unknown;
public:
	virtual caseiteminfo_type GetType() { return type;};
	virtual QString to_string() {
		QString output;
		output.append("IniInfoBase");
		return output;
	}
};
typedef std::shared_ptr<IniInfoBase> IniInfoBasePtr;
class ManualViewInfoBase:public IniInfoBase
{
private:
	caseiteminfo_type type = caseiteminfo_type::ManualConfirmWithRes;
public:
	virtual caseiteminfo_type GetType() { return type; };
	QString raw_str;
	QList<QString> msgs;
	QList<QString> resource;
	virtual QString to_string() {
		QString output;
		//output.append("[msgs] ");output.append(msgs.size());
		foreach(auto msg ,msgs) {
			output.append(msg);
			output.append("\r\n");
		}
#if 0
		output.append("[resource] "); output.append(resource.size());
		foreach(auto msg, resource) {
			resource.append(msg);
			resource.append(";");
		}
#endif
		return output;
	}
};
class NetworkLabelInfoBase:public IniInfoBase
{
private:
	caseiteminfo_type type = caseiteminfo_type::PassconditionWithNetworkId;
public:
	virtual caseiteminfo_type GetType() { return type; };
	QString networklabel;
	virtual QString to_string() {
		QString output;
		output.append(networklabel);
		return output;
	}
};
//typedef std::shared_ptr<NetworkLabelInfoBase> NetworkLabelInfoBasePtr;
class NetworkLabelPreconditionBase:public NetworkLabelInfoBase {
private:
	caseiteminfo_type type = caseiteminfo_type::PreconditionWithNetworkId;
public:
	virtual caseiteminfo_type GetType() { return type; };
	
	int32_t voltage_mv=-1;
	int32_t duration_ms=0;
	virtual QString to_string() {
		QString output;
		output.append(networklabel);
		output.append(QStringLiteral(" 输出 "));
		output.append(QString("%1").arg(voltage_mv));
		output.append(" mV");
		if (duration_ms != 0) {
			output.append(QStringLiteral("持续"));
			output.append(QString("%1").arg(duration_ms/1000));
			output.append(" S");
		}
		return output;
	};
};
typedef std::shared_ptr<NetworkLabelPreconditionBase> NetworkLabelPreconditionBasePtr;
class NetworkLabelPassconditionBase :public NetworkLabelInfoBase
{
public:
	int32_t duration_ms = 0;
	QString mode;
	QString unit; //A or V
	double rangmin_m;
	double rangmax_m;
	virtual QString to_string() {
		QString output;
		output.append(networklabel);
		output.append(QString(" %1").arg(mode));
		output.append(QStringLiteral(" 保持范围 ["));
		output.append(QString("%1").arg(rangmin_m));
		output.append(",");
		output.append(QString("%1").arg(rangmax_m));
		output.append(" ]");
		output.append(unit);
		if (duration_ms != 0) {
			output.append(QStringLiteral(" 持续"));
			output.append(QString("%1").arg(duration_ms / 1000));
			output.append(" S");
		}
		return output;
	};
};
typedef std::shared_ptr<NetworkLabelPassconditionBase> NetworkLabelPassconditionBasePtr;
static std::map<QString, caseitem_class> caseitem_class_map;
using ShowString = QString;
//typedef int32_t (*caseitembase_handle)(std::string);
typedef std::function<int32_t(std::string,int mstep)> caseitembase_handle;

typedef struct __caseitem_handles {
	caseitembase_handle Commenthandle=nullptr;
	caseitembase_handle Ignorehandle = nullptr;
	caseitembase_handle Delaymshandle = nullptr;
	caseitembase_handle ManualNoticehandle = nullptr;
	caseitembase_handle ManualConfirmhandle = nullptr;

	caseitembase_handle Preconditionhandle = nullptr;
	caseitembase_handle Passconditionhandle = nullptr;
}caseitem_handles;
typedef struct __caseitem_handles CaseItemHandles;
typedef struct __caseitem_handles* PCaseItemHandles;

//typedef int32_t(*caseitembase_show)(std::string,std::string &);
typedef std::function<int32_t(std::string, ShowString &)> caseitembase_show;
typedef struct __caseitem_shows {
	caseitembase_show Commentshow = nullptr;
	caseitembase_show Ignoreshow = nullptr;
	caseitembase_show Delaymsshow = nullptr;
	caseitembase_show ManualNoticeshow = nullptr;
	caseitembase_show ManualConfirmshow = nullptr;

	caseitembase_show Preconditionshow = nullptr;
	caseitembase_show Passconditionshow = nullptr;
}caseitem_shows;
typedef struct __caseitem_shows CaseItemShows;
typedef struct __caseitem_shows* PCaseItemShows;


/*
确认参数，执行者，不在乎显示
*/
class CaseItemBase;
typedef std::shared_ptr<CaseItemBase> CaseItemBasePtr;
typedef std::pair<caseitem_class, std::string> CaseItemPair;//有序不唯一
typedef std::vector<CaseItemPair> CaseItemContainer;
using CaseItemBasePtrContain = std::vector<CaseItemBasePtr>;
class CaseItemBase:public QObject
{
	Q_OBJECT
public:
	static CaseItemBasePtr get_instance(QSettings *,int);
	static QString GetVersion() {

		return QString("%1[%2]").arg("CaseItem").arg(CIVERSION);
	};
	CaseItemBase() = delete;
	~CaseItemBase();
	CaseItemBase(CaseItemContainer);

	int32_t GetConditioncnt();
	int32_t GetResultitemcnt();
	int32_t Getitemstotal() {
		return mcaseitems.size();
	};
	QString GetConditionShowTotal();
	int32_t Getitemtypeshow(caseitem_class type, ShowString& output);
	int32_t Getitemshow(int mstep, ShowString& output);
	int32_t Process(int mstep);
	caseitem_type Getitemtype(int mstep);
	bool isCaseIgnore();
signals:
	void notifytoView(int, MessageTVBasePtr);
private:
	static bool checkparam(QSettings *, CaseItemContainer &map);
	static caseitem_class getCaseItemclass(QString input);
	int32_t DefaultCaseItemBaseHandle(std::string input, int mstep = -1);
	int32_t CaseItemCommentHandle(std::string input, int mstep=-1);
	int32_t CaseItemIgnoreHandle(std::string input, int mstep = -1);
	int32_t CaseItemDelaymsHandle(std::string input, int mstep = -1);
	int32_t CaseItemManualNoticeHandle(std::string input, int mstep = -1);
	int32_t CaseItemManualConfirmHandle(std::string input, int mstep = -1);
	int32_t CaseItemPreconditionHandle(std::string input, int mstep = -1);
	int32_t CaseItemPassconditionHandle(std::string input, int mstep = -1);

	int32_t DefaultCaseItemBaseShow(std::string input, ShowString & output);
	int32_t CaseItemIgnoreShow(std::string input, ShowString& output);
	int32_t CaseItemDelaymsShow(std::string input, ShowString & output);
	int32_t CaseItemManualNoticeShow(std::string input, ShowString& output);
	int32_t CaseItemManualConfirmShow(std::string input, ShowString& output);
	int32_t CaseItemPreconditionShow(std::string input, ShowString& output);
	int32_t CaseItemPassconditionShow(std::string input, ShowString& output);

	//QStringList translation_slash(std::string input);
	IniInfoBasePtr translation_slash_smart(std::string input, QString &output, caseitem_class type= caseitem_class::Unknown);
	IniInfoBasePtr PassconditionWithNetworkId(QString input, QString& output);
	IniInfoBasePtr ManualConfirmWithRes(QString input, QString& output);
	IniInfoBasePtr PreconditionWithNetworkId(QString input, QString& output);

	//int32_t translation_slash(std::string input, ShowString& output);
	void RegisterHandles(PCaseItemHandles);
	void RegisterShows(PCaseItemShows);
	int32_t Getitemhandle(int mstep);

	int32_t FunctionSetVoltageOut(int32_t dev_id,NetworkLabelPreconditionBase* msg = nullptr);
	int32_t FunctionQueryCurrent(int32_t dev_id,QString &output, NetworkLabelPassconditionBase* msg = nullptr);
	CaseItemContainer mcaseitems;
	CaseItemHandles mcaseitemhandles;
	CaseItemShows  mcaseItemshows;
	caseitem_typecontainer mtypecontainer;
	int32_t mconditioncnt=-1;
	int32_t mresultitemcnt=-1;
	int moffset_inlist;
	bool misignore = false;
	static CaseItemBasePtrContain mstaticcaseitemptrcontain;
};

