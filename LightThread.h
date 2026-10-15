// Thread.h: interface for the CMyThread class.
//
//////////////////////////////////////////////////////////////////////
/************************************
  Create By: Jerry

************************************/
#ifndef LIGHTTHREAD_H
#define LIGHTTHREAD_H

#include "GeneralConfig.h"
#include "Thread.h"
#include "LightEvent.h"

class OPENLIB_DLL CLightThread : public OS_Thread
{
public:
	CLightThread()
	{
		m_hExitEvent.event_create(false, false);
		m_exitFlag = FLAG_IDLE;
	}

	virtual ~CLightThread()
	{
		StopThread();
		m_hExitEvent.event_destroy();
	}

	int StartThread()
	{
		if (m_exitFlag != FLAG_START)
		{
			m_exitFlag = FLAG_START;

			StartT();
		}

		return 0;
	}
	int StartThread(void* pFun, void* pParam)
	{
		m_exitFlag = FLAG_START;

		StartT(pFun, pParam);
		return 0;
	}
	void StopThread()
	{
		if (m_exitFlag != FLAG_START)
		{
			return;
		}

		//等待处理线程退出。
		m_exitFlag = FLAG_STOPING;	//置进程结束标志

		m_hExitEvent.event_set();
		Join(this);
	}

	int QuitFlag()
	{
		return m_exitFlag;
	}

	//! 设置线程优先级别
	void SetPrority(Prority_Thread nPrority)
	{
		SetProrityofThread(nPrority);
	}

public:
	/* 线程函数 */
	virtual void Run()
	{
		//判断一下是否真“线程退出”。
		while (m_exitFlag != FLAG_STOPING)
		{
			Msleep(1000);
		}
	}

protected:
	enum {
		FLAG_IDLE = 1,//还没有启动.
		FLAG_START,//系统启动
		FLAG_STOPING,//系统要退出了。
		FLAG_STOPED //线程已退出。
	};

	//系统退出标志
	int m_exitFlag;
public:
	CLightEvent m_hThread;
	CLightEvent m_hExitEvent;
};

#endif
