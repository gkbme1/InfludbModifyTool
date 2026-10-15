#pragma once


// CHBExBasicListCtrl
#include <vector>
#include "stdafx.h"

class CHBExBasicListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CHBExBasicListCtrl)

public:
	CHBExBasicListCtrl();
	virtual ~CHBExBasicListCtrl();

	afx_msg void OnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	void OnGetdispinfoDcs(NMHDR *pNMHDR, LRESULT *pResult);

protected:
	void AutoSizeColumns();	//自动列宽
	DECLARE_MESSAGE_MAP()

//public:
//	enumBasicListCtrlType m_enmCtrlType;
private:
	
public:
	COLORREF m_bkClr;
	int m_nItemHeight;//自定义行高

public:
	void InitListCtrl();	//初始化ListCtrl
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);	//ListCtrl自绘函数
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnLvnOdfinditem(NMHDR *pNMHDR, LRESULT *pResult);

	void SetBackColor(COLORREF cr);

};


