#ifndef WINREDEFINE_H
#define WINREDEFINE_H
#include "GeneralConfig.h"
#include "LightThread.h"
#include "LightEvent.h"
#include <string>
#include "LinuxEvent.h"
#include <time.h>
using namespace std;
#ifndef _WINDOWS
typedef struct tagPOINT
{
	long  x;
	long  y;
} POINT;
typedef unsigned long   COLORREF;
typedef unsigned long   COLORREF;
typedef struct tagLOGPEN
{
	unsigned int        lopnStyle;
	POINT       lopnWidth;
	COLORREF    lopnColor;
} LOGPEN;

typedef struct tagLOGBRUSH
{
	unsigned int        lbStyle;
	COLORREF    lbColor;
	unsigned long   lbHatch;
} LOGBRUSH;

typedef struct tagRECT
{
	long    left;
	long    top;
	long    right;
	long    bottom;
} RECT;
typedef int* HDC;
typedef int* HRGN;
typedef int* HFONT;
typedef int*  HWND;
typedef void *HINSTANCE;
typedef void *HANDLE;
typedef int   BOOL;
#define INFINITE            0xFFFFFFFF  // Infinite timeout
/* Hatch Styles */
#define BS_SOLID            0
#define BS_NULL             1
#define BS_HOLLOW           BS_NULL
#define BS_HATCHED          2
#define BS_PATTERN          3
#define BS_INDEXED          4
#define BS_DIBPATTERN       5
#define BS_DIBPATTERNPT     6
#define BS_PATTERN8X8       7
#define BS_DIBPATTERN8X8    8
#define BS_MONOPATTERN      9

#define HS_HORIZONTAL       0       /* ----- */
#define HS_VERTICAL         1       /* ||||| */
#define HS_FDIAGONAL        2       /* \\\\\ */
#define HS_BDIAGONAL        3       /* ///// */
#define HS_CROSS            4       /* +++++ */
#define HS_DIAGCROSS        5       /* xxxxx */
#define HS_API_MAX          12        4
//颜色处理
typedef long LONG;
#define DWORD_PTR unsigned long
typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
typedef unsigned short      WORD;

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))

typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

#if defined(MIDL_PASS)
typedef struct _LARGE_INTEGER {
#else // MIDL_PASS
typedef union _LARGE_INTEGER {
	struct {
		DWORD LowPart;
		LONG HighPart;
	} DUMMYSTRUCTNAME;
	struct {
		DWORD LowPart;
		LONG HighPart;
	} u;
#endif //MIDL_PASS
	LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

unsigned long GetTickCount()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#endif
#ifdef  _WINDOWS
#include <windows.h>
#include <windef.h>

#endif //  _WINDOWS
#define MAX_WAIT_HANDLE_NUM 64
//! 0成功等到所有信号 1超时
extern OPENLIB_DLL unsigned long WaitForMultipleEventsComm(unsigned long nCount, CLightEvent *lpHandles, bool bWaitAll, unsigned long  dwMilliseconds);
extern OPENLIB_DLL  int _beginthreadexComm(
	//void *security,
	//unsigned stacksize,
	OS_TID* tidp,
	void* pFun,//unsigned(__stdcall * initialcode) (void *),
	void * argument,
	void *restrict_attr, //线程属性
	unsigned int createflag = 0,
	unsigned int *thrdaddr = NULL
);
extern OPENLIB_DLL bool clossThreadHandle(OS_TID tidp);
extern OPENLIB_DLL unsigned int GetSystemDirectoryComm(wchar_t*FileName, int ilen);
extern OPENLIB_DLL wchar_t *inet_ntowComm(long lAddr);
extern OPENLIB_DLL unsigned long inet_addrComm(const wstring strInput);
extern OPENLIB_DLL int waitableTimerCallComm(bool bManualReset, long lstartTime, long lintervalTime, void* pFun, void* pParam);
#endif // !WINREDEFINE_H
