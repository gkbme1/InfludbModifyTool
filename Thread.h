#ifndef _OSAPI_THREAD_H
#define _OSAPI_THREAD_H

/*
windows 设置线程优先级一般有五个级别:
	LOWEST			-2
	BELOW_NORMAL	-1
	NORMAL			0
	ABOVE_NORMAL	1
	HIGHEST			2
而linux线程优先级一般在1到99之间
*/

#include "GeneralConfig.h"

#ifndef _WINDOWS
	#include <pthread.h>
#endif

typedef enum Prority_Thread
{
	Lowest = 0,
	Below_Nornal,
	Normal,
	Above_Normal,
	Highest
};

#define THREAD_PRIORITY_LINUX_LOWEST          1
#define THREAD_PRIORITY_LINUX_BELOW_NORMAL    (THREAD_PRIORITY_LINUX_LOWEST+25)
#define THREAD_PRIORITY_LINUX_NORMAL          50
#define THREAD_PRIORITY_LINUX_HIGHEST         99
#define THREAD_PRIORITY_LINUX_ABOVE_NORMAL    (THREAD_PRIORITY_LINUX_HIGHEST-25)

class OPENLIB_DLL OS_Thread
{
public:
	OS_Thread();
	virtual ~OS_Thread();

	// 创建并启动
	virtual int StartT();
	int StartT(void* pFun, void* pParam);
	// 等待和收回资源
	static void Join(OS_Thread* thrd);

	// Sleep函数
	static void Msleep(int ms);
	static void Sleep(int s);

	//! 设置线程优先级别
	virtual void SetProrityofThread(Prority_Thread nPrority);

public:
	virtual void Run() = 0;

public:
	void* m_Priv;

#ifndef _WINDOWS
	pthread_attr_t m_attr;
	bool	m_bHasPrority;
#endif
};
typedef struct OS_Thread_Priv
{
#ifdef _WINDOWS
	void* hThread;
#endif // 

#ifndef _WINDOWS
	pthread_t hThread;
#endif // !_WINDOWS

	
} OS_TID;

#endif