/******************************************************************************
*                                                                             *
*       File Name: GDBCCommonHeaders.h                                        *
*       Description: Include all standar headers of c/c++ and 3er party libs. *
*                                                                             *
*       Author: Keinier Caboverde Ramírez. <keinier@gmail.com>                *
*       Date: 03/05/2013 08:51 a.m.                                           *
*       File's Version: 1.3.1                                                 *
*                                                                             *
******************************************************************************/

#ifndef __GDBC_GDBCCOMMONHEADERS_H__
#define __GDBC_GDBCCOMMONHEADERS_H__

#ifdef __BORLANDC__
    #define __STD_ALGORITHM
#endif

#if defined ( GDBC_GCC_VISIBILITY )
#   pragma GCC visibility push(default)
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <cerrno>
#include <csignal>


// STL containers
#include <vector>
#include <map>
#include <string>
#include <set>
#include <list>
#include <deque>
#include <queue>
//#include <bitset>
#include <sstream>
#include <algorithm>

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#if defined(__sun__)
#include <ieeefp.h> // finite() on Solaris
#endif


#if (GDBC_COMPILER == GDBC_COMPILER_GNUC) && !defined(STLPORT)
#   if GDBC_COMP_VER >= 430
#       include <tr1/unordered_map>
#       include <tr1/unordered_set> 
#   else
#       include <ext/hash_map>
#       include <ext/hash_set>
#   endif
#else
#   if (GDBC_COMPILER == GDBC_COMPILER_MSVC) && !defined(STLPORT) && GDBC_COMP_VER >= 1600 // VC++ 10.0
#    	include <unordered_map>
#    	include <unordered_set>
#	else
#   	include <hash_set>
#   	include <hash_map>
#	endif
#endif 

// STL algorithms & functions
#include <algorithm>
#include <functional>
#include <limits>

// C++ Stream stuff
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

extern "C" {
#   include <sys/types.h>
#   include <sys/stat.h>
}

#if GDBC_PLATFORM == GDBC_PLATFORM_WIN32
#  undef min
#  undef max
#include <WinSock2.h>
#	include <Windows.h>
#  if defined( __MINGW32__ )
#    include <unistd.h>
#  endif
#endif

#if GDBC_PLATFORM == GDBC_PLATFORM_LINUX
extern "C" {
#   include <unistd.h>
#   include <dlfcn.h>
#	include <pthread.h>
}
#endif

#if GDBC_PLATFORM == GDBC_PLATFORM_APPLE || GDBC_PLATFORM == GDBC_PLATFORM_IPHONE
extern "C" {
#   include <unistd.h>
#   include <sys/param.h>
#   include <CoreFoundation/CoreFoundation.h>
}
#endif



#if defined ( GDBC_GCC_VISIBILITY )
#   pragma GCC visibility pop
#endif
#endif // __GDBC_GDBCCOMMONHEADERS_H__