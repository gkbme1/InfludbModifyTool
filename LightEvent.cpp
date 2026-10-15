#include "LightEvent.h"
#include "Thread.h"
#include <iostream>
#include "StdAfx.h"
#ifndef _WINDOWS
	#include <sys/time.h>
	#include <errno.h>
#endif

#define MAX_WAIT_HANDLE_NUM 64

CLightEvent::CLightEvent()
{
	
}

CLightEvent::~CLightEvent()
{

}

void CLightEvent::event_create(bool manual_reset, bool init_state)
{   
#ifdef _WINDOWS
	//! 指定将事件对象创建成手动复原还是自动复原。
	//! 如果是TRUE，那么必须用ResetEvent函数来手工将事件的状态复原到无信号状态。
	//! 如果设置为FALSE，当一个等待线程被释放以后，系统将会自动将事件状态复原为无信号状态。
    hevent = CreateEvent(NULL, manual_reset, init_state, NULL);
#else
    hevent = new(std::nothrow) event_t;
    if(hevent == NULL)
    {
        return;
    }
    hevent->state = init_state;
    hevent->manual_reset = manual_reset;
    if (pthread_mutex_init(&hevent->mutex, NULL))
    {
        delete hevent;
    }
    if (pthread_cond_init(&hevent->cond, NULL))
    {
        pthread_mutex_destroy(&hevent->mutex);
        delete hevent;
    }
#endif
}

void CLightEvent::event_destroy()
{
#ifdef _WINDOWS
    CloseHandle(hevent);
	hevent = NULL;
#else
    pthread_cond_destroy(&hevent->cond);
    pthread_mutex_destroy(&hevent->mutex);
    delete hevent;
	hevent = NULL;
#endif
}

int CLightEvent::event_set()
{
#ifdef _WINDOWS
    return !SetEvent(hevent);
#else
    if (pthread_mutex_lock(&hevent->mutex) != 0)
    {
        return -1;
    }

    hevent->state = true;

    if (hevent->manual_reset)
    {
        if(pthread_cond_broadcast(&hevent->cond))
        {
            return -1;
        }
    }
    else
    {
        if(pthread_cond_signal(&hevent->cond))
        {
            return -1;
        }
    }

    if (pthread_mutex_unlock(&hevent->mutex) != 0)
    {
        return -1;
    }

    return 0;
#endif
}

int CLightEvent::event_reset() 
{
#ifdef _WINDOWS
    //ResetEvent 返回非零表示成功
    if (ResetEvent(hevent))
    {
        return 0;
    } 
    return -1;
#else
    if (pthread_mutex_lock(&hevent->mutex) != 0)
    {
        return -1;
    }

    hevent->state = false;

    if (pthread_mutex_unlock(&hevent->mutex) != 0)
    {      
        return -1;
    }
    return 0;
#endif
}

int CLightEvent::event_wait()
{
#ifdef _WINDOWS
    unsigned long ret = WaitForSingleObject(hevent, INFINITE);
    if (ret == WAIT_OBJECT_0)
    {
        return 0;
    }
    return -1;
#else
    if (pthread_mutex_lock(&hevent->mutex) != 0)   
    {      
        return -1;   
    }   
    while (!hevent->state)    
    {      
        if (pthread_cond_wait(&hevent->cond, &hevent->mutex))   
        {   
            pthread_mutex_unlock(&hevent->mutex); 
            return -1;   
        }   
    }   
    if (!hevent->manual_reset) 
    {
		//! 自动重置
        hevent->state = false;
    }
    if (pthread_mutex_unlock(&hevent->mutex) != 0)   
    {     
        return -1;   
    }  
    return 0;
#endif
}

int CLightEvent::event_timedwait(long milliseconds)
{
#ifdef _WINDOWS
    unsigned long ret = WaitForSingleObject(hevent, milliseconds);
    if (ret == WAIT_OBJECT_0)
    {
        return 0;
    }
    if (ret == WAIT_TIMEOUT)
    {
        return 1;
    }
    return -1;
#else

    int rc = 0;   
    struct timespec abstime;   
    struct timeval tv;   
    gettimeofday(&tv, NULL);   
    abstime.tv_sec  = tv.tv_sec + milliseconds / 1000;   
    abstime.tv_nsec = tv.tv_usec*1000 + (milliseconds % 1000)*1000000;   
    if (abstime.tv_nsec >= 1000000000)   
    {   
        abstime.tv_nsec -= 1000000000;   
        abstime.tv_sec++;   
    }   

    if (pthread_mutex_lock(&hevent->mutex) != 0)   
    {     
        return -1;   
    }   
    while (!hevent->state)    
    {      
        if (rc = pthread_cond_timedwait(&hevent->cond, &hevent->mutex, &abstime))   
        {   
            if (rc == ETIMEDOUT) break;   
            pthread_mutex_unlock(&hevent->mutex);    
            return -1;   
        }   
    }   
    if (rc == 0 && !hevent->manual_reset)   
    {
        hevent->state = false;
    }
    if (pthread_mutex_unlock(&hevent->mutex) != 0)   
    {      
        return -1;   
    }
    if (rc == ETIMEDOUT)
    {
        //timeout return 1
        return 1;
    }
    //wait event success return 0
    return 0;
#endif
}

event_handle CLightEvent::event_gethandle()
{
	return hevent;
}

bool CLightEvent::event_value()
{
	bool ret = false;

#ifndef _WINDOWS
	pthread_mutex_lock(&hevent->mutex);
    ret = hevent->state;
	pthread_mutex_unlock(&hevent->mutex);
#endif

	return ret;
}

