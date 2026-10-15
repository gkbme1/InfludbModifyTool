#include "StdAfx.h"
#include "QueryDDBThread.h"
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
//#include <rapidjson/filereadstream.h>
//#include <rapidjson/document.h>
//#include <rapidjson/stringbuffer.h>
//#include <rapidjson/prettywriter.h>
//using namespace rapidjson;

CQueryDDBThread::CQueryDDBThread(void)
{
	m_pHttpClient_DDB = new CHttpClient();
	m_iIndex = -1;
}


CQueryDDBThread::~CQueryDDBThread(void)
{
}

void CQueryDDBThread::Run()
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


				string strReadDDBUrl1 = g_strIpInfo;
				strReadDDBUrl1 += "user/get_token";
				string strRespond1;
				string strToken = "";
				string json_content1= "";
				int iRet = m_pHttpClient_DDB->LogOn(strReadDDBUrl1,strToken,json_content1,strRespond1);

				if(iRet == 0)
				{
					int iPos = strRespond1.find("token");
					strToken = strRespond1.substr(iPos+8,36);
				}




				string file_name = g_filePath;
				file_name += "\\DDB.json"; //"D:/99se/json.json";
				std::ifstream in(file_name);
				if (!in.is_open())
				{
					fprintf(stderr, "fail to read json file: %s\n", file_name);

				}
				std::string json_content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
				in.close();

				//string strReadDDBUrl = "http://127.0.0.1:9000/ddb/read_ddbtagvalue";
				string strReadDDBUrl = g_strIpInfo;
				strReadDDBUrl += "ddb/read_ddbtagvalue";
				
				string strRespond;
				//string strToken = g_strToken;
				int iRet1 = m_pHttpClient_DDB->Post(strReadDDBUrl,strToken,json_content,strRespond);


				iCurrentTimes = iCurrentTimes+1;
				if(iRet1!=0)
				{
					iErroNum = iErroNum+1;
				}
				if(m_iIndex==0)
				{
					m_pdlg->m_current_Times = iCurrentTimes;
					m_pdlg->m_ErroNum = iErroNum;
					m_pdlg->m_cur_ret = iRet1;
					m_pdlg->m_strResult = strRespond.c_str();
					m_pdlg->UpdateData(0);
				}
			}

	}
}