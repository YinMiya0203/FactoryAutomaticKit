#include "QGlobalConfigDialog.h"
#include <QRadioButton>
#include "GlobalSettings.h"
#include "GlobalConfig.h"

QGlobalConfigDialog::QGlobalConfigDialog(QWidget* parent)
{
	if (!GLOBALSETTINGSINSTANCE->isUserRoot())return;
	auto ptopVBoxLayout = new QVBoxLayout(this);
	mbgGroup = new QButtonGroup(this);
	mbgGroup->setExclusive(false);
#if 1
	#define ADDNEWRBTOGROUP(xxx)	\
		QRadioButton* m##xxx##radioButton = new QRadioButton(QString("GC_debug"#xxx));	\
		ptopVBoxLayout->addWidget(m##xxx##radioButton); \
		mbgGroup->addButton(m##xxx##radioButton);	\
		mbgGroup->setId(m##xxx##radioButton, int(GC_DebugFlags::GC_debug##xxx))
#endif
	ADDNEWRBTOGROUP(thread);
	ADDNEWRBTOGROUP(pbclickevent);
	ADDNEWRBTOGROUP(devciewidget);
	ADDNEWRBTOGROUP(devciedriver);
	ADDNEWRBTOGROUP(devcieBase);
	ADDNEWRBTOGROUP(testcaseini);
	ADDNEWRBTOGROUP(testcaseverbose);
	ADDNEWRBTOGROUP(testcasebgserver);
	ADDNEWRBTOGROUP(testcasetablewidget);
	ADDNEWRBTOGROUP(AutoTestView);
	ADDNEWRBTOGROUP(CaseItemBase);
	ADDNEWRBTOGROUP(FactoryAutoTest);
	ADDNEWRBTOGROUP(GloabalSettings);
	ADDNEWRBTOGROUP(TestCaseResultSave);
#define VIEWRB(xxx) m##xxx##radioButton->setChecked(GlobalConfig_debug##xxx);
	VIEWRB(thread);
	VIEWRB(pbclickevent);
	VIEWRB(devciewidget);
	VIEWRB(devciedriver);
	VIEWRB(devcieBase);
	VIEWRB(testcaseini);
	VIEWRB(testcaseverbose);
	VIEWRB(testcasebgserver);
	VIEWRB(testcasetablewidget);
	VIEWRB(AutoTestView);
	VIEWRB(CaseItemBase);
	VIEWRB(FactoryAutoTest);
	VIEWRB(GloabalSettings);
	VIEWRB(TestCaseResultSave);
	connect(mbgGroup, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
		[=](int id, bool checked) { 
			qDebug("id %d checked %d",id,checked);
			if (id > int(GC_DebugFlags::GC_debugBegin) && id < int(GC_DebugFlags::GC_debugEND)) {
				switch(GC_DebugFlags(id)) {
				case	GC_DebugFlags::GC_debugthread:				GlobalConfig_debugthread = checked; break;
				case	GC_DebugFlags::GC_debugpbclickevent:		GlobalConfig_debugpbclickevent = checked; break;
				case	GC_DebugFlags::GC_debugdevciewidget:		GlobalConfig_debugdevciewidget = checked; break;
				case	GC_DebugFlags::GC_debugdevciedriver:		GlobalConfig_debugdevciedriver = checked; break;
				case	GC_DebugFlags::GC_debugdevcieBase:		    GlobalConfig_debugdevcieBase = checked; break;
				case	GC_DebugFlags::GC_debugtestcaseini:		GlobalConfig_debugtestcaseini = checked; break;
				case	GC_DebugFlags::GC_debugtestcaseverbose:	GlobalConfig_debugtestcaseverbose = checked; break;
				case	GC_DebugFlags::GC_debugtestcasebgserver:	GlobalConfig_debugtestcasebgserver = checked; break;
				case	GC_DebugFlags::GC_debugtestcasetablewidget:GlobalConfig_debugtestcasetablewidget = checked; break;
				case	GC_DebugFlags::GC_debugAutoTestView:		GlobalConfig_debugAutoTestView = checked; break;
				case	GC_DebugFlags::GC_debugCaseItemBase:		GlobalConfig_debugCaseItemBase = checked; break;
				case	GC_DebugFlags::GC_debugFactoryAutoTest:	GlobalConfig_debugFactoryAutoTest = checked; break;
				case	GC_DebugFlags::GC_debugGloabalSettings:	GlobalConfig_debugGloabalSettings = checked; break;
				case	GC_DebugFlags::GC_debugTestCaseResultSave:	GlobalConfig_debugTestCaseResultSave = checked; break;
				default:
					break;
				}
			}
		
		});
}
