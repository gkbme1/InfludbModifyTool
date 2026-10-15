
// TestInfluxDB_2.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号
#include "QueryDDBThread.h"
#include "QueryHDBThread.h"

// CTestInfluxDB_2App:
// 有关此类的实现，请参阅 TestInfluxDB_2.cpp
//

class CTestInfluxDB_2App : public CWinApp
{
public:
	CTestInfluxDB_2App();

// 重写
public:
	virtual BOOL InitInstance();

	void functionaa(time_t starttime,time_t endtime,char chTagName[64],int pix,int iIndex,float fMD,float fMU,char chTagName_temp[64]);
	
// 实现

	DECLARE_MESSAGE_MAP()
};

extern CTestInfluxDB_2App theApp;