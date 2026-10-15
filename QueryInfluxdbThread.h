#pragma once
#include "LightEvent.h"
#include "LightThread.h"
#include "LightMutex.h"
//#include "TestInfluxDB_2Dlg.h"

enum{
	STATE_IDLE = 10,
	STATE_REBUILDING ,
	STATE_PROCESSING ,
	STATE_STOPED 
};

class CQueryInfluxdbThread :
	public CLightThread
{
public:
	CQueryInfluxdbThread(void);
	~CQueryInfluxdbThread(void);

	int m_iState;//线程状态
	//起始时间 结束时间
	time_t m_starttime;
	time_t m_endtime;
	int m_pix;
	//线程主循环（是可以重载的虚函数）
	virtual void Run();
	char m_chTagName[64];
	CLightEvent m_hReadEvent;//
	CLightEvent m_hReadEventOK;

	//界面显示使用
	int m_iIndex;
	void SaveOneTag_3Month(int Index);
	void SaveOneTag_4Month(int Index);


	int m_iStartProcessPointIndex;
	int m_iStartProcessDayIndex;
};

