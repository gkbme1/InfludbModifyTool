

#include "StdAfx.h"

//#include "TestInfluxDB_2Dlg.h"
#include "QueryInfluxdbThread.h"

#include "WinRedefine.h"
#include "TestInfluxDB_2.h"
#include <vector>
using namespace std;

extern CTestInfluxDB_2App theApp;
extern BOOL g_bExit;
extern vector<NeedModifyPoints*> g_AllTagListInfo;
extern time_t g_StartTime;

extern int g_CurrentProcessPointNumIndex;
extern int g_CurrentProcessDayIndex;


CQueryInfluxdbThread::CQueryInfluxdbThread(void)
{
	//m_pInfluxDB_2Dlg = NULL;
	m_iState = STATE_IDLE;
	memset(m_chTagName,0,64);
	m_hReadEvent.event_create(false, false);
	m_hReadEventOK.event_create(false, false);

	m_iStartProcessPointIndex = 0;
	m_iStartProcessDayIndex = 0;

}


CQueryInfluxdbThread::~CQueryInfluxdbThread(void)
{
	if (m_hReadEventOK.event_gethandle())
	{
		m_hReadEventOK.event_destroy();
	}
	if (m_hReadEvent.event_gethandle())
	{
		m_hReadEvent.event_destroy();
	}
}

//add by gaozuhan below
void CQueryInfluxdbThread::Run()
{
	//判断一下是否真“线程退出”。
	//while (1)
	{
		CLightEvent hEv[3];

		CLightEvent hExitEvent;
		hEv[0] = m_hReadEvent;
		hEv[1] = hExitEvent;
	    bool bIsFind = false;
		for(int cc = 0;cc<g_AllTagListInfo.size();cc++)		
		{
			int iPointIndex = g_AllTagListInfo[cc]->iPointIndex;
			int iIndex_Day = g_AllTagListInfo[cc]->iIndex_Day;
			if(!bIsFind)
			{
				if(iPointIndex>= m_iStartProcessPointIndex && iIndex_Day>=m_iStartProcessDayIndex)
				{
					bIsFind = true;
				}
			}

			if(bIsFind)
			{
				time_t StartTime = g_StartTime+(g_AllTagListInfo[cc]->iIndex_Day-1)*24*60*60;
				time_t EndTime = StartTime+24*60*60;

				int Index = 0;
				float fMD = 0.0;
				float fMU = 0.0;
				fMD = g_AllTagListInfo[cc]->fMiddleValue- g_AllTagListInfo[cc]->fOffet;
				fMU = g_AllTagListInfo[cc]->fMiddleValue+ g_AllTagListInfo[cc]->fOffet;

				g_CurrentProcessPointNumIndex = iPointIndex;
				g_CurrentProcessDayIndex = iIndex_Day;

				theApp.functionaa(StartTime,EndTime,g_AllTagListInfo[cc]->chOldPointName,86400,Index,fMD,fMU,g_AllTagListInfo[cc]->chNewPointName);

					g_AllTagListInfo[cc];
					char chDebug[1000] = {0};
					sprintf(chDebug,"gao0708 cc:%d DayIndex = %d TagName:%s-%s Middle:%f Offset:%f",cc,
						g_AllTagListInfo[cc]->iIndex_Day,g_AllTagListInfo[cc]->chNewPointName,g_AllTagListInfo[cc]->chOldPointName,
						g_AllTagListInfo[cc]->fMiddleValue,g_AllTagListInfo[cc]->fOffet);
					OutputDebugStringA(chDebug);
			}

			if(g_bExit)
			{
				break;
			}

		}

		
	}
}

void CQueryInfluxdbThread::SaveOneTag_3Month(int Index)
{

	//for(m_iIndex = 9;m_iIndex<10;m_iIndex++)
	{
		//m_iIndex =1;
		switch(Index)
		{
		case 1:
			{
				//sprintf(m_chTagName,"GDY050103002");
			}
			break;
		case 2:
			{
				//sprintf(m_chTagName,"GDY050103003");
			}
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			break;
		default:
			break;
		}

		int ww = 5;
		//if(Index==7)
		//{
		//	ww = 5;
		//}
		
		for(;ww<49;ww++)
		{


			float fMD = 0;
			float fMU = 0;
			float fMiddle = 0;
			float fOffet = 0;
			switch(ww)
			{
			case 1:
			case 2:
				{
					fMiddle = 200;
					fOffet = 19.8;
				}
				break;
			case 3:
			case 4:
				{
					fMiddle = 215;
					fOffet = 19.8;
				}
				break;
			case 5:
			case 6:
				{
					fMiddle = 230;
					fOffet = 19.8;
				}
				break;
			case 7:
			case 8:
				{
					fMiddle = 245;
					fOffet = 19.8;
				}
				break;
			case 9:
			case 10:
				{
					fMiddle = 260;
					fOffet = 19.8;
				}
				break;
			case 11:
			case 12:
				{
					fMiddle = 290;
					fOffet = 19.8;
				}
				break;
			case 13:
			case 14:
				{
					fMiddle = 320;
					fOffet = 19.8;
				}
				break;
			case 15:
			case 16:
				{
					fMiddle = 350;
					fOffet = 19.8;
				}
				break;
			case 17:
			case 18:
				{
					fMiddle = 390;
					fOffet = 19.8;
				}
				break;
			case 19:
			case 20:
				{
					fMiddle = 430;
					fOffet = 19.8;
				}
				break;
			case 21:
			case 22:
				{
					fMiddle = 480;
					fOffet = 19.8;
				}
				break;
			case 23:
			case 24:
				{
					fMiddle = 530;
					fOffet = 19.8;
				}
				break;
			case 25:
			case 26:
				{
					fMiddle = 580;
					fOffet = 19.8;
				}
				break;
			case 27:
			case 28:
				{
					fMiddle = 630;
					fOffet = 19.8;
				}
				break;
			case 29:
			case 30:
				{
					fMiddle = 680;
					fOffet = 19.8;
				}
				break;
			case 31:
			case 32:
				{
					fMiddle = 720;
					fOffet = 19.8;
				}
				break;
			case 33:
			case 34:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 35:
			case 36:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 37:
			case 38:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 39:
			case 40:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 41:
			case 42:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 43:
			case 44:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 45:
			case 46:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 47:
			case 48:
				{
					fMiddle = 727;
					fOffet = 9.8;
				}
				break;
			default:
				break;
			}
			fMD = fMiddle-fOffet;
			fMU = fMiddle+fOffet;
			char m_chTagName_temp[64] = {0};

			sprintf(m_chTagName,"GDY050%d0300%d_JRZ%02dTI01_AV",Index>6?2:1,Index>6?(Index-6):Index,ww);
			sprintf(m_chTagName_temp,"GDY050%d0300%d_JRZ%02dTI01_A",Index>6?2:1,Index>6?(Index-6):Index,ww);

			CTime mytime(2024,3,1,0,0,0);
			time_t StartTime = mytime.GetTime();
			//一次查两天数据，查完直接插入


			char chDebug[500]={0};

			sprintf(chDebug,"gao0510 m_iIndex:%d ww:%d m_chTagName:%s %s start",Index,ww,m_chTagName,m_chTagName_temp);
			OutputDebugStringA(chDebug);
			time_t EndTime = 0;

			//StartTime = StartTime+29*24*60*60;
			for(int i=0;i<32;i++)
			{

				EndTime = StartTime+24*60*60;

				theApp.functionaa(StartTime,EndTime,m_chTagName,86400,Index,fMD,fMU,m_chTagName_temp);

				StartTime = EndTime;

				sprintf(chDebug,"gao0510 m_iIndex:%d ww:%d day:%d end",Index,ww,i);
				OutputDebugStringA(chDebug);
			}

			sprintf(chDebug,"gao0510 m_iIndex:%d ww:%d m_chTagName:%s %s end -------",Index,ww,m_chTagName,m_chTagName_temp);
			OutputDebugStringA(chDebug);
		}


	}

}


void CQueryInfluxdbThread::SaveOneTag_4Month(int Index)
{

	//for(m_iIndex = 9;m_iIndex<10;m_iIndex++)
	{
		//m_iIndex =1;
		switch(Index)
		{
		case 1:
			{
				//sprintf(m_chTagName,"GDY050103002");
			}
			break;
		case 2:
			{
				//sprintf(m_chTagName,"GDY050103003");
			}
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			break;
		default:
			break;
		}

		int ww = 4;
		//if(Index==1)
		//{
		//	ww = 30;
		//}
		if(Index==5)
		{
			ww = 38;
		}

		//ww = 5;
		for(;ww<49;ww++)
		{


			float fMD = 0;
			float fMU = 0;
			float fMiddle = 0;
			float fOffet = 0;
			switch(ww)
			{
			case 1:
			case 2:
				{
					fMiddle = 200;
					fOffet = 19.8;
				}
				break;
			case 3:
			case 4:
				{
					fMiddle = 215;
					fOffet = 19.8;
				}
				break;
			case 5:
			case 6:
				{
					fMiddle = 230;
					fOffet = 19.8;
				}
				break;
			case 7:
			case 8:
				{
					fMiddle = 245;
					fOffet = 19.8;
				}
				break;
			case 9:
			case 10:
				{
					fMiddle = 260;
					fOffet = 19.8;
				}
				break;
			case 11:
			case 12:
				{
					fMiddle = 290;
					fOffet = 19.8;
				}
				break;
			case 13:
			case 14:
				{
					fMiddle = 320;
					fOffet = 19.8;
				}
				break;
			case 15:
			case 16:
				{
					fMiddle = 350;
					fOffet = 19.8;
				}
				break;
			case 17:
			case 18:
				{
					fMiddle = 390;
					fOffet = 19.8;
				}
				break;
			case 19:
			case 20:
				{
					fMiddle = 430;
					fOffet = 19.8;
				}
				break;
			case 21:
			case 22:
				{
					fMiddle = 480;
					fOffet = 19.8;
				}
				break;
			case 23:
			case 24:
				{
					fMiddle = 530;
					fOffet = 19.8;
				}
				break;
			case 25:
			case 26:
				{
					fMiddle = 580;
					fOffet = 19.8;
				}
				break;
			case 27:
			case 28:
				{
					fMiddle = 630;
					fOffet = 19.8;
				}
				break;
			case 29:
			case 30:
				{
					fMiddle = 680;
					fOffet = 19.8;
				}
				break;
			case 31:
			case 32:
				{
					fMiddle = 720;
					fOffet = 19.8;
				}
				break;
			case 33:
			case 34:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 35:
			case 36:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 37:
			case 38:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 39:
			case 40:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 41:
			case 42:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 43:
			case 44:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 45:
			case 46:
				{
					fMiddle = 747;
					fOffet = 0.8;
				}
				break;
			case 47:
			case 48:
				{
					fMiddle = 727;
					fOffet = 9.8;
				}
				break;
			default:
				break;
			}
			fMD = fMiddle-fOffet;
			fMU = fMiddle+fOffet;
			char m_chTagName_temp[64] = {0};

			sprintf(m_chTagName,"GDY050%d0300%d_JRZ%02dTI01_AV",Index>6?2:1,Index>6?(Index-6):Index,ww);
			sprintf(m_chTagName_temp,"GDY050%d0300%d_JRZ%02dTI01_A",Index>6?2:1,Index>6?(Index-6):Index,ww);

			CTime mytime(2024,3,1,0,0,0);
			time_t StartTime = mytime.GetTime();
			//一次查两天数据，查完直接插入


			char chDebug[500]={0};

			sprintf(chDebug,"gao0510 m_iIndex:%d ww:%d m_chTagName:%s %s start",Index,ww,m_chTagName,m_chTagName_temp);
			OutputDebugStringA(chDebug);
			time_t EndTime = 0;

			StartTime = StartTime+31*24*60*60;
			for(int i=31;i<71;i++)
			{

				EndTime = StartTime+24*60*60;

				theApp.functionaa(StartTime,EndTime,m_chTagName,86400,Index,fMD,fMU,m_chTagName_temp);

				StartTime = EndTime;

				if(g_bExit)
				{
					//软件退出
					return;
				}
				sprintf(chDebug,"gao0510 m_iIndex:%d ww:%d day:%d end",Index,ww,i);
				OutputDebugStringA(chDebug);
			}

			sprintf(chDebug,"gao0510 m_iIndex:%d ww:%d m_chTagName:%s %s end -------",Index,ww,m_chTagName,m_chTagName_temp);
			OutputDebugStringA(chDebug);
		}


	}

}
//add by gaozuhan above