#pragma once
#include "LightEvent.h"
#include "LightThread.h"
#include "LightMutex.h"
#include "HttpClient.h"
class CQueryHDBThread :
	public CLightThread
{
public:
	CHttpClient* m_pHttpClient_HDB;
	CQueryHDBThread(void);
	~CQueryHDBThread(void);
	//线程主循环（是可以重载的虚函数）
	virtual void Run();
	int m_iIndex;
};

