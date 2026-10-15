/*!
 *	\file	OS_Ext.h
 *
 *	\brief	屏蔽平台差异代码头文件
 *
 *	对于ACE没有封装的底层API在此文件中进行封装，同时包括一些屏蔽平台差异的代码
 *
 *	$Header: /MACS-SCADA_SUD/OORTDB_Design/Server/PublicFile/OS_Ext/OS_Ext.h 35
 *	$Author:
 *	$Date: 09/11/20 20:41 $
 *	$Revision: 35 $
 *
 *	\attention	Copyright (c) 2002-2008, MACS-SCADA System Development, HollySys Co.,Ltd
 *	\attention	All rights reserved.
 */
#ifndef _OS_EXT_H__
#define _OS_EXT_H__

#pragma warning(disable:4786)

#include <string.h>

//! 只定义了WIN32而没有定义_WINDOWS
#ifdef WIN32
#ifndef _WINDOWS
#define _WINDOWS
#endif
#endif

//#define snprintf  _sntprintf

////////////////////////////////////////////////////////////////////
//! WINDOWS专用
#ifdef _WINDOWS

//! 支持_T字符操作宏
#include <tchar.h>

//#define snprintf _snprintf

#define MACS_DLL_EXPORT __declspec(dllexport)
#define MACS_DLL_IMPORT __declspec(dllimport)


//! 获取缺省ini文件存放目录，Windows下缺省ini文件存放在Windows目录下
#ifndef GetDefaultIniDirectory
#define GetDefaultIniDirectory GetWindowsDirectory
#endif

#define _MODULE_PREFIX ""
#define _MODULE_POSTFIX ".dll"
#define _EXE_POSTFIX ".exe"

//! longlong类型调用sprintf输出的格式定义
#define PRINT_FORMAT_LONGLONG "I64d"	//! %d == %i
#define PRINT_FORMAT_ULONGLONG "I64u"

#ifndef MAX_PATH
	#define MAX_PATH (260)
#endif

#endif

////////////////////////////////////////////////////////////////////
//! UNIX系列专用
#ifndef _WINDOWS
//#include "WinRedefine.h"
#define MACS_DLL_EXPORT
#define MACS_DLL_IMPORT
//! hash_map等标准模板库为扩展内容，在不同平台下实现在不同命名空间内
#define STL_EXT	__gnu_cxx
//#include <ext/hash_map>

#define _MAX_PATH 260
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_EXT 256
#define _MAX_FNAME 255
#define MAX_PATH 260
#define _tcsncpy wcsncpy
#define _tcslen wcslen
#define stricmp wcscasecmp
#define _TSCHAR wchar_t
#define __int64 long long
#define TCHAR char
#define _tcschr wcschr
#define _stprintf swprintf
#define _ttol _wtoi
#define _ttoi64 _wtoll
#define _tcstod wcstod
#define _ttoi wtoi
#define _sntprintf _snwprintf
#define _tcstol wcstol
#define _tcscmp wcscmp
#define _tcscat wcscat
#define _atoi64 _wtoll

#define __time64_t time_t
#define _time64 time
#define _localtime64 localtime

#define _I64_MIN    LONG_LONG_MIN
#define _I64_MAX    LONG_LONG_MAX
#define _UI32_MAX ULONG_MAX
#define _I32_MIN LONG_MIN

/* Minimum and maximum macros */
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef _T
#define _T(x) L ## x
#endif

#define _MODULE_PREFIX "lib"
#define _MODULE_POSTFIX ".so"
#define _EXE_POSTFIX ""

//! longlong类型调用sprintf输出的格式定义
#define PRINT_FORMAT_LONGLONG "lld" 
#define PRINT_FORMAT_ULONGLONG "llu" 

extern int __argc;
extern char **__argv;
void _splitpath( const char *path, char *drive, char *dir, char *fname, char *ext );
void _makepath(char *path, const char *drive, const char *dir, const char *fname, const char *ext);

//! 获取缺省ini文件存放目录，Unix下缺省ini文件存放在根目录"/"下
int GetDefaultIniDirectory(
	wchar_t * lpBuffer,
	unsigned int uSize);

/*!
 *	\brief	Convert an integer to a string
 *
 *	详细的函数说明（可选）
 *
 *	\param	value: Number to be converted
 *	\param	str: String result
 *	\param	radix: Base of value; must be in the range 2 to 36
 *
 *	\retval	returns a pointer to string
 *
 *	\note	Windows下的itoa函数在Unix/Linux的替代实现
 */
char* itoa(int value, wchar_t *str, int radix);

/*!
 *	\brief	Reverse characters of a string
 *
 *	\param	str: Null-terminated string to reverse
 *
 *	\retval	returns a pointer to the altered string
 *
 *	\note	Windows下的strrev函数在Unix/Linux的替代实现
 */
char* strrev(wchar_t* str);

/*!
 *	\brief	Get the number of milliseconds since boot time
 *
 *	\note	Windows下的GetTickCount函数在Unix/Linux的替代实现
 */
unsigned long GetTickCount();

void OutputDebugString(const wchar_t* lpOutputString);

/*!
*	\brief	文件拷贝
*
*	\param	srcFileName: [in] 源文件名
*
*	\param	dstFileName: [in] 目标文件名
*
*	\param	bFailIfExists: [in] If this parameter is true and the new file specified by dstFileName already exists, the function fails. 
*                               If this parameter is false and the new file already exists, the function overwrites the existing file and succeeds. 
*
*	\retval	If the function succeeds, the return value is 1.
*               else the return value is 0.
*
*	\note	Windows下的CopyFile函数在Unix/Linux的替代实现
*/
int CopyFile(const wchar_t *srcFileName, const wchar_t *dstFileName, bool bFailIfExists);

#endif

////////////////////////////////////////////////////////////////////
#ifndef COMMONBASECODE_API
#ifdef DBCOMMON_EXPORTS
#define COMMONBASECODE_API MACS_DLL_EXPORT
#else
#define COMMONBASECODE_API MACS_DLL_IMPORT
#endif
#endif

/*!
 *	\brief	获取磁盘空间剩余大小，以1M字节为单位
 *
 *	提供OS无关的获取磁盘剩余空间大小函数封装，供应用层调用，属于OS封装层
 *
 *	\param	dir_name Pointer to a null-terminated string that specifies a directory on the disk of interest. 
			If dir_name is NULL, the DiskFreeSpace function will fail. 
			Note that dir_name does not have to specify the root directory on a disk. 
			The function accepts any directory on the disk. 

 *	\retval	unsigned long long, 成功返回以1MB为单位的剩余空间大小，
 *								失败则抛出std::runtime_error( _T("DiskFreeSpace error!\n") );
 *
 *	\note	1 Byte=8 bits, 1 KB=1024 Bytes, 1 MB=1024 KB
 */
const unsigned long DISK_MEGA_BYTE = 1024*1024L;
extern COMMONBASECODE_API unsigned __int64 DiskFreeSpace(const wchar_t*  dir_name);

/*!
 *	\brief	修改系统时钟
 *
 *	\param	modTime, 64位整数,值为Unix Time time_t*10000000+usec
 *
 *	\retval	int, 成功0，失败-1
 */
extern COMMONBASECODE_API int SetClockTime(__int64 modTime);

#endif
