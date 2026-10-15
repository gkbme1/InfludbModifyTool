#ifndef LIGHTMUTEX_H
#define LIGHTMUTEX_H

#ifdef _WINDOWS
	typedef void * Mutex_Handle;
#else
    #include <pthread.h>
	typedef pthread_mutex_t Mutex_Handle;
#endif

class  CLightThreadMutex
{
public:
	CLightThreadMutex();
	~CLightThreadMutex();

	int Lock();
	void Unlock();

private:
	Mutex_Handle m_hMutex;
};

#endif