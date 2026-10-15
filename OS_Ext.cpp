/*!
*	\file	OS_Ext.cpp
*
*	\brief	屏蔽平台差异代码实现文件
*
*	以下部分API的实现是从VC.Net2003平台下拷贝而来，在Unix、Linux使用时应进行测试
*
*	$Header: /MACS-SCADA_SUD/OORTDB_Design/Server/PublicFile/OS_Ext/OS_Ext.cpp
*	$Author: 
*	$Date: 10-10-09 2:09p $
*	$Revision: 38 $
*
*	\attention	Copyright (c) 2002-2008, MACS-SCADA System Development, HollySys Co.,Ltd
*	\attention	All rights reserved.
*/
#include "OS_Ext.h"
#include <stdexcept>

////////////////////////////////////////////////////////////////////
#ifdef _WINDOWS
//! 以下代码只用于Windows平台

#include <Windows.h>

#endif	//#ifdef _WINDOWS

////////////////////////////////////////////////////////////////////
#ifndef _WINDOWS

//! 以下代码只用于非Windows平台
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#ifdef _MBCS
#include <mbstring.h>
#include <mbctype.h>
#include <mbdata.h>
#endif  /* _MBCS */

int __argc;
char **__argv;

/***
*_splitpath() - split a path name into its individual components
*
*Purpose:
*       to split a path name into its individual components
*
*Entry:
*       path  - pointer to path name to be parsed
*       drive - pointer to buffer for drive component, if any
*       dir   - pointer to buffer for subdirectory component, if any
*       fname - pointer to buffer for file base name component, if any
*       ext   - pointer to buffer for file name extension component, if any
*
*Exit:
*       drive - pointer to drive string.  Includes ':' if a drive was given.
*       dir   - pointer to subdirectory string.  Includes leading and trailing
*           '/' or '\', if any.
*       fname - pointer to file base name
*       ext   - pointer to file extension, if any.  Includes leading '.'.
*
*Exceptions:
*
*******************************************************************************/
void _splitpath (
				 const char *path,
				 char *drive,
				 char *dir,
				 char *fname,
				 char *ext
				 )
{
	char *p;
	char *last_slash = NULL, *dot = NULL;
	unsigned len;

	/* we assume that the path argument has the following form, where any
	* or all of the components may be missing.
	*
	*  <drive><dir><fname><ext>
	*
	* and each of the components has the following expected form(s)
	*
	*  drive:
	*  0 to _MAX_DRIVE-1 characters, the last of which, if any, is a
	*  ':'
	*  dir:
	*  0 to _MAX_DIR-1 characters in the form of an absolute path
	*  (leading '/' or '\') or relative path, the last of which, if
	*  any, must be a '/' or '\'.  E.g -
	*  absolute path:
	*      \top\next\last\     ; or
	*      /top/next/last/
	*  relative path:
	*      top\next\last\  ; or
	*      top/next/last/
	*  Mixed use of '/' and '\' within a path is also tolerated
	*  fname:
	*  0 to _MAX_FNAME-1 characters not including the '.' character
	*  ext:
	*  0 to _MAX_EXT-1 characters where, if any, the first must be a
	*  '.'
	*
	*/

	/* extract drive letter and :, if any */

	if ((_tcslen(path) >= (_MAX_DRIVE - 2)) && (*(path + _MAX_DRIVE - 2) == _T(':'))) {
		if (drive) {
			_tcsncpy(drive, path, _MAX_DRIVE - 1);
			*(drive + _MAX_DRIVE-1) = _T('\0');
		}
		path += _MAX_DRIVE - 1;
	}
	else if (drive) {
		*drive = _T('\0');
	}

	/* extract path string, if any.  Path now points to the first character
	* of the path, if any, or the filename or extension, if no path was
	* specified.  Scan ahead for the last occurence, if any, of a '/' or
	* '\' path separator character.  If none is found, there is no path.
	* We will also note the last '.' character found, if any, to aid in
	* handling the extension.
	*/

	for (last_slash = NULL, p = (_TSCHAR *)path; *p; p++) {
#ifdef _MBCS
		if (_ismbblead(*p))
			p++;
		else {
#endif  /* _MBCS */
			if (*p == _T('/') || *p == _T('\\'))
				/* point to one beyond for later copy */
				last_slash = p + 1;
			else if (*p == _T('.'))
				dot = p;
#ifdef _MBCS
		}
#endif  /* _MBCS */
	}

	if (last_slash) {

		/* found a path - copy up through last_slash or max. characters
		* allowed, whichever is smaller
		*/

		if (dir) {
			len = __min((unsigned)(((char *)last_slash - (char *)path) / sizeof(_TSCHAR)),
				(_MAX_DIR - 1));
			_tcsncpy(dir, path, len);
			*(dir + len) = _T('\0');
		}
		path = last_slash;
	}
	else if (dir) {

		/* no path found */

		*dir = _T('\0');
	}

	/* extract file name and extension, if any.  Path now points to the
	* first character of the file name, if any, or the extension if no
	* file name was given.  Dot points to the '.' beginning the extension,
	* if any.
	*/

	if (dot && (dot >= path)) {
		/* found the marker for an extension - copy the file name up to
		* the '.'.
		*/
		if (fname) {
			len = __min((unsigned)(((char *)dot - (char *)path) / sizeof(_TSCHAR)),
				(_MAX_FNAME - 1));
			_tcsncpy(fname, path, len);
			*(fname + len) = _T('\0');
		}
		/* now we can get the extension - remember that p still points
		* to the terminating nul character of path.
		*/
		if (ext) {
			len = __min((unsigned)(((char *)p - (char *)dot) / sizeof(_TSCHAR)),
				(_MAX_EXT - 1));
			_tcsncpy(ext, dot, len);
			*(ext + len) = _T('\0');
		}
	}
	else {
		/* found no extension, give empty extension and copy rest of
		* string into fname.
		*/
		if (fname) {
			len = __min((unsigned)(((char *)p - (char *)path) / sizeof(_TSCHAR)),
				(_MAX_FNAME - 1));
			_tcsncpy(fname, path, len);
			*(fname + len) = _T('\0');
		}
		if (ext) {
			*ext = _T('\0');
		}
	}
}

int GetDefaultIniDirectory(
						   char * lpBuffer,
						   unsigned int uSize)
{
	const char os_dir[] = "/";

	//! Unix下缺省ini文件存放在根目录"/"下
	if ( uSize < strlen(os_dir)+1 )
		return -1;

	strcpy(lpBuffer, os_dir);
	return 0;
}

/***
*void _makepath() - build path name from components
*
*Purpose:
*       create a path name from its individual components
*
*Entry:
*       _TSCHAR *path  - pointer to buffer for constructed path
*       _TSCHAR *drive - pointer to drive component, may or may not contain
*                     trailing ':'
*       _TSCHAR *dir   - pointer to subdirectory component, may or may not include
*                     leading and/or trailing '/' or '\' characters
*       _TSCHAR *fname - pointer to file base name component
*       _TSCHAR *ext   - pointer to extension component, may or may not contain
*                     a leading '.'.
*
*Exit:
*       path - pointer to constructed path name
*
*Exceptions:
*
*******************************************************************************/
void _makepath (
				char *path,
				const char *drive,
				const char *dir,
				const char *fname,
				const char *ext
				)
{
	register const _TSCHAR *p;

	/* we assume that the arguments are in the following form (although we
	* do not diagnose invalid arguments or illegal filenames (such as
	* names longer than 8.3 or with illegal characters in them)
	*
	*  drive:
	*      A           ; or
	*      A:
	*  dir:
	*      \top\next\last\     ; or
	*      /top/next/last/     ; or
	*      either of the above forms with either/both the leading
	*      and trailing / or \ removed.  Mixed use of '/' and '\' is
	*      also tolerated
	*  fname:
	*      any valid file name
	*  ext:
	*      any valid extension (none if empty or null )
	*/

	/* copy drive */

	if (drive && *drive) {
		*path++ = *drive;
		*path++ = _T(':');
	}

	/* copy dir */

	if ((p = dir) && *p) {
		do {
			*path++ = *p++;
		}
		while (*p);
#ifdef _MBCS
		if (*(p=_mbsdec(dir,p)) != _T('/') && *p != _T('\\')) {
#else  /* _MBCS */
		if (*(p-1) != _T('/') && *(p-1) != _T('\\')) {
#endif  /* _MBCS */
			*path++ = _T('\\');
		}
	}

	/* copy fname */

	if (p = fname) {
		while (*p) {
			*path++ = *p++;
		}
	}

	/* copy ext, including 0-terminator - check to see if a '.' needs
	* to be inserted.
	*/

	if (p = ext) {
		if (*p && *p != _T('.')) {
			*path++ = _T('.');
		}
		while (*path++ = *p++)
			;
	}
	else {
		/* better add the 0-terminator */
		*path = _T('\0');
	}
}

char* strrev(char* szT)
{
	int len = strlen(szT);
    char* temp = new char[len];
    
    strncpy(temp, szT, len);
    for(int i = 0; i < len; i++)
    {
        szT[i] = temp[len - i -1];
    }
    
    szT[len] = '\0';

	if(NULL != temp)
	{
		delete[] temp;
		temp = NULL;
	}
    return szT;
}

char* itoa(int value, char*  str, int radix)
{
	int  rem = 0;
	int  pos = 0;
	char ch  = '!' ;
	bool bNegtive = false;

	if ( value < 0)
	{
		bNegtive = true;
		value = -value;			
	}

	do
	{
		rem    = value % radix ;
		value /= radix;
		if ( 16 == radix )
		{
			if( rem >= 10 && rem <= 15 )
			{
				switch( rem )
				{
				case 10:
					ch = 'a' ;
					break;
				case 11:
					ch ='b' ;
					break;
				case 12:
					ch = 'c' ;
					break;
				case 13:
					ch ='d' ;
					break;
				case 14:
					ch = 'e' ;
					break;
				case 15:
					ch ='f' ;
					break;
				}
			}
		}
		if( '!' == ch )
		{
			str[pos++] = (char) ( rem + 0x30 );
		}
		else
		{
			str[pos++] = ch ;
		}
	}while( value != 0 );

	if ( bNegtive )
	{
		str[pos++] = '-' ;
	}	

	str[pos] = '\0' ;
	return strrev(str);
}

unsigned long GetTickCount()
{
	struct timespec ts;  
	
	clock_gettime(CLOCK_MONOTONIC, &ts);  
	
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);  


	/*
	struct tms tm;
	return times(&tm);
	*/
}

void OutputDebugString(const char* lpOutputString)
{
	std::cout << lpOutputString <<std::endl;
}

unsigned __int64 DiskFreeSpace(const char*  dir_name)
{
	// 获取磁盘信息.dir_name为要获得目录路径 
	struct statvfs fs; 
	if( statvfs(dir_name, &fs) == -1 )  
	{ 
		// 失败则抛出异常
		std::string err_msg( strerror(errno) );                     

		std::runtime_error( err_msg.c_str() );
		return 0;  
	}  

	/*!
	The statvfs structure pointed to by buf includes the following members:
	u_long      f_bsize;             //* preferred file system block size
	u_long      f_frsize;            //* fundamental filesystem block(size if supported)
	fsblkcnt_t  f_blocks;            //* total # of blocks on file system in units of f_frsize
	fsblkcnt_t  f_bfree;             //* total # of free blocks
	fsblkcnt_t  f_bavail;            //* # of free blocks avail to non-privileged user
	fsfilcnt_t  f_files;             //* total # of file nodes (inodes)
	fsfilcnt_t  f_ffree;             //* total # of free file nodes
	fsfilcnt_t  f_favail;            //* # of inodes avail to non-privileged user
	u_long      f_fsid;              //* file system id (dev for now)
	char        f_basetype[FSTYPSZ]; //* target fs type name, null-terminated
	u_long      f_flag;              //* bit mask of flags
	u_long      f_namemax;           //* maximum file name length
	char        f_fstr[32];          //* file system specific string
	u_long      f_filler[16];        //* reserved for future expansion
	*/

	unsigned __int64 free_disk = (unsigned __int64)fs.f_bfree*fs.f_frsize/DISK_MEGA_BYTE;

	return free_disk; 
}

int SetClockTime(__int64 modTime)
{
	timeval testTime;

	testTime.tv_sec=(time_t)(modTime/10000000);
	testTime.tv_usec=(long)((modTime%10000000)*100/1000);
	/*
	//修改高精度时间
	timespec newTime;
	newTime.tv_sec=(time_t)(modTime/10000000);
	newTime.tv_nsec=(long)((modTime%10000000)*100);

	if( ACE_OS::clock_settime(CLOCK_REALTIME,&newTime)!=0 )
	*/
	if( settimeofday(&testTime,NULL)!=0 )
	{
		std::string err_msg( strerror(errno) );

		std::runtime_error( err_msg.c_str() );
		return -1;
	}		

	return 0;
}


int CopyFile(const char *srcFileName, const char *dstFileName, bool bFailIfExists)
{
	int infile, outfile;
	int nread;
	const int BUFSIZE = 1024;
	char buffer[BUFSIZE];
	int  perm = 0777; //授予所有权限

	if (bFailIfExists)
	{
		//若目标文件已存在，则返回错误
		if ((outfile = open(dstFileName, O_RDONLY)) != -1)
		{
			close(outfile);
			return 0;
		}
	} 

	////////////////////////////////////////////////////////////
	// 打开源文件
	if ((infile = open(srcFileName, O_RDONLY)) == -1)
		return 0;

	/* 创建并打开目标文件 */
	if ((outfile = open(dstFileName, O_WRONLY|O_CREAT|O_TRUNC, perm) ) == -1)
	{
		close(infile);
		return 0;
	}


	/* 循环的把源文件写入目标文件 */
	while ((nread = read(infile, buffer, BUFSIZE)) > 0)
	{
		if (write(outfile, buffer, nread) < nread)

		{
			nread = -1;
			break;
		}
	}

	/* 关闭资源 */
	close(infile);
	close(outfile);

	if (nread == -1)
		return 0;
	else
		return 1;
}

#endif //#ifndef _WINDOWS

