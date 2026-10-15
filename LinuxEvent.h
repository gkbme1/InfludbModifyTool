#pragma once
#ifndef __LIBROOT_MY_EVENTS_H_
#define __LIBROOT_MY_EVENTS_H_

#ifndef WAIT_TIMEOUT
#include <errno.h>
#define WAIT_TIMEOUT ETIMEDOUT
#endif
#include "WinRedefine.h"
#include <stdint.h>
#include <string>
using namespace std;
//namespace MY_ENVENT
//{
#ifdef _WINDOWS
typedef HANDLE  HEVENT;
#else
	//Type declarations
struct my_event_t_;
typedef my_event_t_ * HEVENT;
#endif

//WIN32-style functions
HEVENT CreateEventComm(bool manualReset = false, bool initialState = false, const wstring& strEventName = L"");
int DestroyEvent(HEVENT event);
int WaitForEvent(HEVENT event, uint64_t milliseconds = -1);
int SetEventComm(HEVENT event);
int ResetEventComm(HEVENT event);

int WaitForMultipleEvents(HEVENT *events, int count, bool waitAll, uint64_t milliseconds);
int WaitForMultipleEvents(HEVENT *events, int count, bool waitAll, uint64_t milliseconds, int &index);

int WaitForMultipleEventsEx(HEVENT *events, int count, bool waitAll, uint64_t milliseconds, bool bAlertable);

#ifdef PULSE
int PulseEvent(HEVENT event);
#endif

//}

#endif
