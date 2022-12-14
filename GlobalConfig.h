#pragma once
extern bool GlobalConfig_debugthread;
extern bool GlobalConfig_debugpbclickevent;
extern bool GlobalConfig_debugdevciewidget;
extern bool GlobalConfig_debugdevciedriver;
extern bool GlobalConfig_debugdevcieBase;
extern bool GlobalConfig_debugtestcaseini;
extern bool GlobalConfig_debugtestcaseverbose;
extern bool GlobalConfig_debugtestcasebgserver;
extern bool GlobalConfig_debugtestcasetablewidget;
extern bool GlobalConfig_debugAutoTestView;
extern bool GlobalConfig_debugCaseItemBase;
extern bool GlobalConfig_debugFactoryAutoTest;
extern bool GlobalConfig_debugGloabalSettings;
extern bool GlobalConfig_debugTestCaseResultSave;

enum class GC_DebugFlags {
	GC_debugBegin,
	GC_debugthread,
	GC_debugpbclickevent,
	GC_debugdevciewidget,
	GC_debugdevciedriver,
	GC_debugdevcieBase,
	GC_debugtestcaseini,
	GC_debugtestcaseverbose,
	GC_debugtestcasebgserver,
	GC_debugtestcasetablewidget,
	GC_debugAutoTestView,
	GC_debugCaseItemBase,
	GC_debugFactoryAutoTest,
	GC_debugGloabalSettings,
	GC_debugTestCaseResultSave,
	GC_debugEND,
};