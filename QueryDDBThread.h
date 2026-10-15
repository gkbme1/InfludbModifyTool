#pragma once
#include "LightEvent.h"
#include "LightThread.h"
#include "LightMutex.h"
#include "HttpClient.h"
class CQueryDDBThread :
	public CLightThread
{
public:
	CHttpClient* m_pHttpClient_DDB;
	CQueryDDBThread(void);
	~CQueryDDBThread(void);
	//线程主循环（是可以重载的虚函数）
	virtual void Run();
	int m_iIndex;

};

