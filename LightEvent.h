#ifndef LIGHTEVENT_H
#define LIGHTEVENT_H

#include "GeneralConfig.h"

#ifdef _WINDOWS
#include <Windows.h>
#define event_handle HANDLE
#else
#include <pthread.h>
typedef struct
{
	bool state;
	bool manual_reset;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}event_t;
#define event_handle event_t*
#endif

const int Event_Result_OK = 0;
const int Event_Result_TimeOut = 1;
const int Event_Result_Error = -1;

class OPENLIB_DLL CLightEvent
{
public:
	CLightEvent();
	~CLightEvent();

	//返回值：NULL 出错
	void event_create(bool manual_reset, bool init_state);

	//返回值：无
	void event_destroy();

	//返回值：0 成功，-1出错
	int event_set();

	//返回值：0 成功，-1出错
	int event_reset();

	//返回值：0 等到事件，-1出错
	int event_wait();

	//返回值：0 等到事件，1 超时，-1出错
	int event_timedwait(long milliseconds);

	//! 获取Handle句柄
	event_handle event_gethandle();

	//! 获取信号是否有效
	bool event_value();

private:
	event_handle  hevent;
};

#endif
