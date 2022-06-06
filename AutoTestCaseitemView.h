#pragma once
#include <cstdint>
#include "CaseItemBase.h"
class AutoTestCaseitemView;
typedef std::shared_ptr<AutoTestCaseitemView> AutoTestCaseitemViewPtr;
typedef std::vector<int32_t> ItemViewContainer;
typedef std::vector<AutoTestCaseitemViewPtr> TestCaseitemcontainer;
class AutoTestCaseitemView
{
	static int32_t ITEM_COLUMSPAN;
	static int32_t INDEX_COLUMSPAN;
	static int32_t UITLEG_COLUMSPAN;
	static int32_t COMMENT_ROWSPAN;
public:
	static AutoTestCaseitemViewPtr get_instance(int mstep, int raw_row);
public:
	AutoTestCaseitemView() = delete;
	AutoTestCaseitemView(int mstep,int raw_row);
	
	int32_t GetindexRect(int32_t &row,int32_t &colum,int32_t &rowspan,int32_t &columspan);
	int32_t GetuitlegcontionRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan);
	int32_t GetuitlegresultRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan);
	int32_t GetcommentRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan);
	int32_t GetcurrentCursorRect(int32_t& row, int32_t& colum, int32_t& rowspan, int32_t& columspan);

	int32_t GetSeekRec(int32_t seek,int32_t& row, int32_t& colum);
	int32_t GetRowSpan();
	int32_t GetColumSpan() {
		return ITEM_COLUMSPAN + INDEX_COLUMSPAN + UITLEG_COLUMSPAN;
	};
	int32_t GetCurrentCursor() {
		return mcursor;
	};
	//int32_t GetCurrentitemShow(std::string &output);
	//int32_t GetCurrentitemtcclass(caseitem_class &type);
	int32_t GetCurrentitemtctype(caseitem_type &type);
	int32_t CursorNext();
	void ResetCursor();
private:

	void setupview();
private:
	void SetCursor(int32_t cursor);
	int32_t mbase_row;
	int32_t mstep;
	int32_t mconditioncnt;
	int32_t mresultcnt;
	int32_t mRowSpan = -1;

	int32_t mcursor = 0;
	int32_t mcursor_max = 0;
	ItemViewContainer mconditionview;
	ItemViewContainer mresultview;
	static TestCaseitemcontainer mstaticcaseitemcontainer;
};

