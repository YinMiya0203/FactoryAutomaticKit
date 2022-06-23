#include <QDateTime>
#include <QTextStream>
#include "TestCaseResultSave.h"
#include "TestCase.h"
#define TIMESTR_STYLE_YMD "yyyy-MM-dd"
#define TIMESTR_STYLE_hsm "hh-mm-ss"
TestCaseResultSaveICMPPtr TestCaseResultSaveICMP::__instance = NULL;
TestCaseResultSaveICMP::TestCaseResultSaveICMP()
{

}
TestCaseResultSaveICMP::~TestCaseResultSaveICMP() 
{
	Utility::OutputDebugPrintf("~TestCaseResultSaveICMP");
	SyncToDisk();
}
TestCaseResultSaveICMPPtr TestCaseResultSaveICMP::get_instance()
{
	if (!__instance)
	{
		TestCaseResultSaveICMPPtr ptr(new TestCaseResultSaveICMP);
		__instance = ptr;
	}
	return __instance;
}

int32_t TestCaseResultSaveICMP::Save(int sector, int seek, QString prex, QString value, QColor c)
{
	int ret = 0;
	if (isSaveAsTable()) {
		ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
		goto ERROR_OUT;
	}
	if (prex.size() == 0)prex = "NULL";
	if (mresultfile == nullptr) {
		mresultfile = std::shared_ptr<ResultFileInfo>(new ResultFileInfo(prex));
		return mresultfile->Fill(sector,seek,prex, value,c);
	}
	if (mresultfile->isFull() ){
		mresultfile = NULL;
		mresultfile = std::shared_ptr<ResultFileInfo>(new ResultFileInfo(prex));

		return mresultfile->Fill(sector, seek, prex, value, c);
	}
	ret =  mresultfile->Fill(sector, seek, prex, value, c);
ERROR_OUT:
	return ret;
}
int32_t TestCaseResultSaveICMP::Save(QTableWidget* table, QString prex)
{
	int ret = 0;
	if (!isSaveAsTable()) {
		ret = -ERROR_DEVICE_FEATURE_NOT_SUPPORTED;
		goto ERROR_OUT;
	}
	if (prex.size() == 0)prex = "NULL";
	if (mresultfile == nullptr) {
		mresultfile = std::shared_ptr<ResultFileInfo>(new ResultFileInfo(prex));
		return mresultfile->Fill(table, prex);
	}
	if (mresultfile->isFull()) {
		mresultfile = NULL;
		mresultfile = std::shared_ptr<ResultFileInfo>(new ResultFileInfo(prex));

		return mresultfile->Fill(table, prex);
	}
	ret = mresultfile->Fill(table, prex);
ERROR_OUT:
	return ret;
}
int32_t TestCaseResultSaveICMP::SyncToDisk()
{
	mresultfile.reset();;
	return 0;
}

ResultFileInfo::ResultFileInfo(QString prex):
	mfilledcnt(0)
{
	if(GlobalConfig_debugTestCaseResultSave)qDebug(" %p", this);

}

ResultFileInfo::~ResultFileInfo()
{
	Utility::OutputDebugPrintf("~ResultFileInfo");
	if (mfile.fileName().size() <= 0 ||  !mfile.isOpen()) {
		if (GlobalConfig_debugTestCaseResultSave)qDebug("empty file");
		return;
	}
	//close file rename file
	{	
		auto mtime = QDateTime::currentDateTime();
		QString current_dir = mtime.toString(TIMESTR_STYLE_YMD);
		QString Log_dir = mcurrentlogdir;
		QString raw_log_file = QString("%1/%7/%2-%3_%4-%5_%6-Result.html").	\
			arg(Log_dir).	\
			arg(mStarttime.toString("hhmmss")).	\
			arg(mtime.toString("hhmmss")).	\
			arg(mStartPrex).	\
			arg(mcurrentprex).	\
			arg(mfilledcnt). \
			arg(current_dir);
		FillEnd(raw_log_file);
		mfile.rename(raw_log_file);
		mfile.close();
	}
}

int32_t ResultFileInfo::Fill(int sector, int seek, QString prex, QString value, QColor c)
{
	int ret = 0;

	//first
	if (!mfile.isOpen()) {
		ret = SetupNewfile();
		if (ret != 0)goto ERROR_OUT;
		//heard
		ret = FillHead();
		if (ret != 0)goto ERROR_OUT;
		{
			QTextStream nstream(&mfile);
			nstream << QString("<table width = \"100%\" style = \"word-break;break-all;background:#f2f2f2\">") << endl;
			//操作步骤 做好tabel 表头
			int total_step = TestcaseBase::get_instance()->GettestcaseStepcnt();
			nstream << QString("<tr>") << endl;
			for (int i = 0; i < total_step; i++) {
				if (i == 0) {
					nstream << QString("<th> Index </th>") << endl;
				}
				nstream << QString("<th>");
				{
					int passitems = TestcaseBase::get_instance()->Gettestcasetresultitemcnt(i);
					nstream << QString("<td colspan =\"%1\">").arg(passitems);
					QString raw = TestcaseBase::get_instance()->GettestcasetConditionShowTotal(i);

					nstream << raw;
					nstream << QString("</td>") << endl;
				}
				nstream << QString("</th>") << endl;
			}
			nstream << QString("</tr>") << endl;		
		}
		mStartPrex = prex;
	}
	{
		QString raw_out;
		ret = FormatTable(sector,seek,prex,value,c, raw_out);
		if(ret!=0){		
			ret = -ERROR_INVALID_PARAMETER;
			goto ERROR_OUT;
		}
		QTextStream mstream(&mfile);
		mstream << raw_out << endl;
	}

	mfilledcnt++;
	mcurrentprex = prex;
ERROR_OUT:

	return ret;
}

int32_t ResultFileInfo::Fill(QTableWidget* table, QString prex)
{
	//
	int ret = 0;
	//first
	if (!mfile.isOpen()) {
		ret = SetupNewfile();
		if (ret != 0)goto ERROR_OUT;
		//heard
		ret = FillHead();
		if (ret != 0)goto ERROR_OUT;
		mStartPrex = prex;
	}
	{
		QTextStream nstream(&mfile);
		nstream << QString("<table width = \"100%\" style = \"word-break;break-all;background:#f2f2f2\">") << endl;
		QString raw_out;
		ret = FormatTable(table, prex, raw_out);
		if (ret != 0) {
			ret = -ERROR_INVALID_PARAMETER;
			goto ERROR_OUT;
		}
		nstream << raw_out << endl;
		nstream << "</table>" << endl;

	}

	mfilledcnt++;
	mcurrentprex = prex;
ERROR_OUT:
	if (ret != 0) {
		qCritical("ret 0x % x",ret);
	}
	return ret;
}

int32_t ResultFileInfo::FillEnd(QString filename)
{
	int ret = 0;
	if (mfile.fileName().size()<=0 || !mfile.isOpen()) {
		//qDebug("empty file");
		goto ERROR_OUT;
	}
	{
		QString name = QFileInfo(filename).baseName();
		QTextStream mstream(&mfile);
		if (mCurrentSector >= 0) { 
			mstream << QString("</tr>") << endl;
			mstream << QString("</table>") << endl;
		}

		mstream << QString("</body>") << endl;
		mstream << QString("<title>") << endl;
		mstream << name << endl;
		mstream << QString("</title>") << endl;
		mstream << QString("</html>") << endl;
	}
ERROR_OUT:
	return ret;
}

int32_t ResultFileInfo::FormatTable(int sector, int seek, QString prex, QString value, QColor c, QString& output)
{
	int ret = 0;
	if (mfile.fileName().size() <= 0 || !mfile.isOpen()) {
		//qDebug("empty file");
		goto ERROR_OUT;
	}
	if (sector==0&&seek==0) {
		//fill index
		output.append(QString("<tr>"));
		output.append(QString("<td>"));
		output.append(prex);
		output.append(QString("</td>"));
		output.append("\r\n");
	}
	if (mCurrentSector!= sector&& mCurrentSector>=0) {
		output.append(QString("</tr>"));
		output.append("\r\n");
	}
	output.append(QString::asprintf("<td bgcolor=\"%s\" >", c.name()));
	if (GlobalConfig_debugTestCaseResultSave)qDebug("c %s",c.name());
	output.append(value);
	output.append(QString("</td>"));
	output.append("\r\n");
	mCurrentSector = sector;

ERROR_OUT:
	return ret;
}

int32_t ResultFileInfo::FormatTable(QTableWidget* tableWidget, QString prex,QString& output)
{
	int ret = 0;
	output.clear();
	if (mfile.fileName().size() <= 0 || !mfile.isOpen()) {
		//qDebug("empty file");
		goto ERROR_OUT;
	}
	{
		int row_count = tableWidget->rowCount(); int col_count = tableWidget->columnCount();
		if (GlobalConfig_debugTestCaseResultSave)qDebug("Table [%d ~ %d]", row_count, col_count);
		output.append(QString("<caption> %1 </caption>\r\n").arg(prex));
		// calc width
		QVector<int> rect_list;
		{
			for (int col = 0; col < col_count; col++) {
				auto item = tableWidget->item(1, col);
				auto rect = tableWidget->visualItemRect(item);
				//qDebug("col %d:%d", col, rect.width());
				rect_list.push_back(rect.width());
			}        int total = 0;
			foreach(auto w, rect_list) {
				total += w;
			}
			for (int col = 0; col < col_count; col++) {

				rect_list[col] = (rect_list[col] * 100 / total);
				if (rect_list[col]==0) {
					rect_list[col] = 5;
				}
			}
		}
		
		for (int row = 0; row < row_count; row++) {
			output.append(QString("<tr>\r\n"));
			int32_t next_vail_col = 0;
			for (int col = 0; col < col_count; col++) {

				auto item = tableWidget->item(row, col);
				QString td_raw="";
				int row_span = tableWidget->rowSpan(row, col);
				int col_span = tableWidget->columnSpan(row, col);

				//qDebug("rc %d,%d ----span %d %d",row,col, tableWidget->rowSpan(row,col), tableWidget->columnSpan(row, col));
				if (item == nullptr/*|| next_vail_col> col*/) {
					//qDebug("rc %d,%d visual %d %d next_vail_col %d", row, col, tableWidget->visualRow(row), tableWidget->visualColumn(col), next_vail_col);
					continue;
				}
				if (col_count >= next_vail_col) {
					next_vail_col = col + col_span;
					//qDebug("next_vail_col update %d %d %d", next_vail_col, col, col_span);
				}
				auto rect = tableWidget->visualItemRect(item);
				//qDebug("rect %s %d,%d", item->text().toStdString().c_str(), rect.width(), rect.height());

				//if (item == nullptr) { td_raw = "<td>"; goto NULL_ITEM; }
				if(/*row == 0*/true) {
					auto color = item->backgroundColor();

					if (color.alpha()!=0) {
						td_raw = QString::asprintf("<td rowspan=\"%d\" colspan=\"%d\" bgcolor=\"%s\" width=\"%d%%\">", row_span, col_span,
							color.name().toStdString().c_str(), rect_list[col]);
					}
					else {
						td_raw = QString::asprintf("<td rowspan=\"%d\" colspan=\"%d\" width=\"%d%%\">", row_span, col_span, rect_list[col]);
					}
					
				}
				else {
					auto color = item->backgroundColor().name();

					if (color != "#000000") {
						td_raw = QString::asprintf("<td rowspan=\"%d\" colspan=\"%d\" bgcolor=\"%s\">", row_span, col_span,
							color.toStdString().c_str());
					}
					else {
						td_raw = QString::asprintf("<td rowspan=\"%d\" colspan=\"%d\">", row_span, col_span);
					}
				
				}
				td_raw.append(item->text());
				output.append(td_raw);
				output.append(QString("</td>\r\n"));
			}
			output.append(QString("</tr>\r\n"));
		}
	}
ERROR_OUT:
	return ret;
}

int32_t ResultFileInfo::SetupNewfile()
{
	int ret = 0;
	mStarttime = QDateTime::currentDateTime();
	QString current_dir = mStarttime.toString(TIMESTR_STYLE_YMD);
	QString current_name = mStarttime.toString(TIMESTR_STYLE_hsm);
	QString Log_dir = GLOBALSETTINGSINSTANCE->GetTestCaseResultDirLocation();
	mcurrentlogdir = Log_dir;
	QString raw_log_file = QString("%1/%2/.%3-tmp.html").arg(Log_dir).arg(current_dir).arg(current_name);
	mfile.setFileName(raw_log_file);
	qInfo("raw_log_file %s", raw_log_file.toStdString().c_str());
	Utility::NewDir(QFileInfo(raw_log_file).absolutePath());
	if (!mfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qCritical("open %s fail", mfile.fileName().toStdString().c_str());
		ret = -ERROR_FILE_INVALID;
		goto ERROR_OUT;
	};
ERROR_OUT:
	return ret;
}

int32_t ResultFileInfo::FillHead()
{
	int ret = 0;
	if (!mfile.isOpen()) {
		ret  = SetupNewfile();
		if (ret != 0)goto ERROR_OUT;
	}
	{
		QTextStream mstream(&mfile);
		mstream << QString("<!DOCTYPE html>") << endl;
		mstream << QString("<html>") << endl;
		mstream << QString("<head>") << endl;
		mstream << QString("<style>")<<endl;
		mstream << QString("table, table td, table th{ border:1px solid; border-collapse:collapse }");
		mstream << QString("</style>") << endl;
		mstream << QString("<meta charset=\"ASCI\">") << endl;
		mstream << QString("</head>") << endl;
		mstream << QString("<body>") << endl;
		mstream << QString("<text>") << endl;
		mstream << QStringLiteral("测试员:") << endl;
		mstream << GLOBALSETTINGSINSTANCE->GetCurrentOp() << endl;
		mstream << QString("</text> <br>") << endl;
	}
ERROR_OUT:
	return ret;
}
