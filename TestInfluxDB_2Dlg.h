
// TestInfluxDB_2Dlg.h : 头文件
//
#include "HttpClient.h"
#include "string.h"
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "curl/curl.h"


#include "InfluxdbClient.h"

using namespace std;
#pragma once


#include "QueryInfluxdbThread.h"
#include "HBExBasicListCtrl.h"



// CTestInfluxDB_2Dlg 对话框
//class CQueryInfluxdbThread;
class CTestInfluxDB_2Dlg : public CDialogEx
{
// 构造
public:
	enum { IDD = IDD_TESTINFLUXDB_2_DIALOG };
	CTestInfluxDB_2Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	
	void QuerySinglePointTrend(time_t starttime,time_t endtime,char chTagName[32],int pix,int iIndex);

	void QuerySinglePointTrendEX(time_t starttime,time_t endtime,char chTagName[64],int pix,int iIndex,float fMD,float fMU,char chTagName_temp[64]);


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void ProcessOneData(int &CurrentRowNum, int iAllColNum,int iAllRowNum, char ** charTemStringBuffer,NeedModifyPoints  **pNeedModifyPointList);
	void ReadTagsFromCSV();

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CHttpClient* m_pHttpClient;
	InfluxdbClient* m_pClient;
	afx_msg void OnBnClickedButton2();

	int m_pix;
	CListCtrl m_ListCtrl;
	CListCtrl m_ListCtrl2;
	CListCtrl m_ListCtrl3;

	CHBExBasicListCtrl			m_ListMacsPoint;		//Macs点列表



	CQueryInfluxdbThread m_QueryInfluxdbThread[MAXIMUM_WAIT_OBJECTS];
	void ShowResultToList(int iIndex,PointValue *pPointValue,int iReadTagNum);

	afx_msg void OnBnClickedQuery222Data();
	CString m_Result;
//	int m_cur_ret;
//	int m_current_Times;
//	int m_ErroNum;
	int m_cur_ret;
	int m_ErroNum;
//	CString m_current_Times;
	int m_current_Times;
	CString m_strResult;
	afx_msg void OnClose();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	int m_ProcessAllDays;
	int m_ProcessAllPointNum;
	int m_StartProcesPointIndex;
	int m_StartProcessDayIndex;
	afx_msg void OnBnClickedStartProcess();
	CButton m_Start_Stop_Process;

	bool m_bIsInProcessing;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
