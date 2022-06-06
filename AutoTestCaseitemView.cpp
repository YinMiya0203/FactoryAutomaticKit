#include "TestCase.h"
#include "AutoTestCaseitemView.h"

int32_t AutoTestCaseitemView::ITEM_COLUMSPAN = 8;
int32_t AutoTestCaseitemView::INDEX_COLUMSPAN = 1;
int32_t AutoTestCaseitemView::UITLEG_COLUMSPAN = 1;
int32_t AutoTestCaseitemView::COMMENT_ROWSPAN = 1;
#define CNT_TO_ROW(x)	(((x) + (ITEM_COLUMSPAN - 1)) / ITEM_COLUMSPAN)

TestCaseitemcontainer AutoTestCaseitemView::mstaticcaseitemcontainer = {};
AutoTestCaseitemViewPtr AutoTestCaseitemView::get_instance(int mstep, int raw_row)
{
	if (mstep < mstaticcaseitemcontainer.size()) {
		return mstaticcaseitemcontainer[mstep];
	}
	if(mstep>(TestcaseBase::get_instance()->GettestcaseStepcnt()-1)){
		return nullptr;
	}
	AutoTestCaseitemViewPtr ptr(new AutoTestCaseitemView(mstep, raw_row));
	mstaticcaseitemcontainer.push_back(ptr);
	return ptr;
}


void AutoTestCaseitemView::ResetCursor()
{
	mcursor = 0;
}
void AutoTestCaseitemView::setupview() 
{
	mconditionview.clear();
	mresultview.clear();
	int total = TestcaseBase::get_instance()->Gettestcaseitemstotal(mstep);
	mcursor_max = total;
	for (int i = 0; i < total; i++) {
		caseitem_type type = TestcaseBase::get_instance()->Gettestcaseitemtype(mstep,i);
		if (type == caseitem_type::Contition){
			mconditionview.push_back(i);
		}
		else if (type == caseitem_type::ResultData) {
			mresultview.push_back(i);
		}
	}
}

AutoTestCaseitemView::AutoTestCaseitemView(int mstep,int raw_row):
	mbase_row(raw_row), mstep(mstep),mconditioncnt(0),mresultcnt(0)
{
	ResetCursor();
	GetRowSpan();
	setupview();
}

void AutoTestCaseitemView::SetCursor(int32_t cursor)
{
	do {
		mcursor = cursor;
	} while (0);
}
int32_t AutoTestCaseitemView::GetindexRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan)
{
	int ret = 0;
	row = mbase_row;
	colum = 0;
	rowspan = GetRowSpan();
	columspan = 1;
	return ret;
}

int32_t AutoTestCaseitemView::GetuitlegcontionRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan)
{
	int ret = 0;
	row = mbase_row+ COMMENT_ROWSPAN;
	colum = INDEX_COLUMSPAN;
	rowspan = CNT_TO_ROW(mconditioncnt);
	columspan = UITLEG_COLUMSPAN;
	return ret;
}
int32_t AutoTestCaseitemView::GetuitlegresultRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan)
{
	int ret = 0;
	row = mbase_row + COMMENT_ROWSPAN+ CNT_TO_ROW(mconditioncnt);
	colum = INDEX_COLUMSPAN;
	rowspan = CNT_TO_ROW(mresultcnt);
	columspan = UITLEG_COLUMSPAN;
	return ret;
}

int32_t AutoTestCaseitemView::GetcommentRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan)
{
	int ret = 0;
	row = mbase_row;
	colum = INDEX_COLUMSPAN+ UITLEG_COLUMSPAN;
	rowspan = 1;
	columspan = ITEM_COLUMSPAN;
	return ret;
}
int32_t AutoTestCaseitemView::GetcurrentCursorRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan)
{
	int ret = 0;
	caseitem_type type;
	int offset = GetCurrentitemtctype(type);
	switch (type) {
		case caseitem_type::Contition:
			row = mbase_row + COMMENT_ROWSPAN;
			colum = INDEX_COLUMSPAN + UITLEG_COLUMSPAN;
			row += (offset/ITEM_COLUMSPAN);
			colum += (offset% (ITEM_COLUMSPAN));
			break;
		case caseitem_type::ResultData:
			row = mbase_row + COMMENT_ROWSPAN+ CNT_TO_ROW(mconditioncnt);
			colum = INDEX_COLUMSPAN + UITLEG_COLUMSPAN;
			row += (offset / ITEM_COLUMSPAN);
			colum += (offset % (ITEM_COLUMSPAN));
			break;
			
		default:
			ret = -1;
			break;
	}
	rowspan = 1;
	columspan = 1;
	if(GlobalConfig_debugtestcasetablewidget)qDebug("view: mcursor %d type %d ,offset %d to [%d-%d]",mcursor,type,offset,row,colum);
	return ret;
}

int32_t AutoTestCaseitemView::GetCurrentitemtctype(caseitem_type& type)
{
	int ret = -1;
	if (std::count(mconditionview.begin(), mconditionview.end(), mcursor) != 0){
		type = caseitem_type::Contition;
		ret = std::find(mconditionview.begin(), mconditionview.end(), mcursor) - mconditionview.begin();
	}
	else if (std::count(mresultview.begin(), mresultview.end(), mcursor) != 0) {
		type = caseitem_type::ResultData;
		ret = ret = std::find(mresultview.begin(), mresultview.end(), mcursor) - mresultview.begin();;
	}
	return ret;
}
int32_t AutoTestCaseitemView::GetSeekRec(int32_t seek,int32_t& row, int32_t& colum)
{
	int ret = 0;
	int32_t rowspan, columspan;
	SetCursor(seek);
	ret = GetcurrentCursorRect(row, colum, rowspan, columspan);
	return ret;
}
int32_t AutoTestCaseitemView::CursorNext()
{
	int ret = -1;
	if (mcursor<mcursor_max) {
		mcursor++; 
	}
		return ret;
}
int32_t AutoTestCaseitemView::GetRowSpan()
{
	if (mRowSpan >= 0)return mRowSpan;

	int row_total = 0;
	row_total += COMMENT_ROWSPAN;
	int row = TestcaseBase::get_instance()->Gettestcasetconditionitemcnt(mstep);
	mconditioncnt = row;
	row = CNT_TO_ROW(row);
	row_total += row;
	row = TestcaseBase::get_instance()->Gettestcasetresultitemcnt(mstep);
	mresultcnt = row;
	row = CNT_TO_ROW(row);
	row_total += row;

	mRowSpan = row_total;
	return row_total;
}


