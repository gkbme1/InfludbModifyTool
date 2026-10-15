#include "StdAfx.h"
#include "QueryHDBThread.h"

#include "WinRedefine.h"
#include "TestInfluxDB_2.h"
#include "TestInfluxDB_2Dlg.h"

extern CTestInfluxDB_2App theApp;
extern string g_filePath;
extern string g_strToken;
extern CTestInfluxDB_2Dlg *m_pdlg;
extern string g_strIpInfo;
#include <fstream>

#include <iostream>


CQueryHDBThread::CQueryHDBThread(void)
{
	m_pHttpClient_HDB = new CHttpClient();
	m_iIndex = -1;
}


CQueryHDBThread::~CQueryHDBThread(void)
{
}

void CQueryHDBThread::Run()
{
	//判断一下是否真“线程退出”。

		
	int iCurrentTimes = 0;
	int iErroNum = 0;
	while (1)
	{
		CLightEvent hEv[3];

		hEv[0] = m_hExitEvent;
		int timer = 2000;
		DWORD waitResult = WaitForMultipleEventsComm(1, hEv, FALSE, timer);
		if (waitResult == WAIT_OBJECT_0)
			break;//thread exit
		else
			if (waitResult == WAIT_TIMEOUT)
			{

				time_t currenttime = time(NULL);
				string file_name = g_filePath;
				file_name += "\\HDB.json"; //"D:/99se/json.json";
				std::ifstream in(file_name);
				if (!in.is_open())
				{
					fprintf(stderr, "fail to read json file: %s\n", file_name);

				}
				std::string json_content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
				in.close();

				//string strReadDDBUrl = "http://127.0.0.1:9000/ddb/read_ddbtagvalue";
				string strReadDDBUrl = g_strIpInfo;
				strReadDDBUrl += "hdb/read_hdbtagvalue";
				
				string strRespond;
				string strToken = g_strToken;
				int iTick1 = GetTickCount();
				int iRet = m_pHttpClient_HDB->Post(strReadDDBUrl,strToken,json_content,strRespond);
				int iTick2 = GetTickCount();

				iCurrentTimes = iCurrentTimes+1;
				if(iRet!=0)
				{
					iErroNum = iErroNum+1;
				}
				if(m_iIndex==0)
				{
					m_pdlg->m_current_Times = iCurrentTimes;
					m_pdlg->m_ErroNum = iTick2-iTick1;//iErroNum;
					m_pdlg->m_cur_ret = iRet;
					m_pdlg->m_strResult = strRespond.c_str();
					m_pdlg->UpdateData(0);
				}
			}

	}


}