/******************************************************************************
*                                                                             *
*       File Name: GDBCPrerequisites.h                                        *
*       Description: Define all directive and configurations of Platform and  *
*                    developer's preference.                                  *
*                                                                             *
*       Author: Keinier Caboverde Ramírez. <keinier@gmail.com>                *
*       Date: 03/05/2013 08:51 a.m.                                           *
*       File's Version: 1.3.1                                                 *
*                                                                             *
******************************************************************************/

#ifndef __GDBC_GDBCPREREQUISITES_H__
#define __GDBC_GDBCPREREQUISITES_H__

/*
	Directiva para el caso de que exista una configuración personalizada
*/
#ifdef HAVE_CONFIG
#	include "SystemConfiguration.h"
#endif

// Directiva de inclución para la detección de la plataforma, en tiempo de compilación y desarrollo
#include "GDBCPlatform.h"
#include "GDBCCommonHeaders.h"
// Configuración para el trazado de memoria
#if SYSTEM_DEBUG_MODE 
#	if SYSTEM_MEMORY_TRACKER_DEBUG_MODE
#		define SYSTEM_MEMORY_TRACKER 1
#	else
#		define SYSTEM_MEMORY_TRACKER 0
#	endif
#else
#	if SYSTEM_MEMORY_TRACKER_RELEASE_MODE
#		define SYSTEM_MEMORY_TRACKER 1
#	else
#		define SYSTEM_MEMORY_TRACKER 0
#	endif
#endif



FORCEINLINE char * gdbc_strdup(const char * source)
{
    char * dest = new char[strlen(source) + 1];
    strcpy(dest, source);
    return dest;
}

/*
	Medidas a tomar en caso de que el compilador sea de Microsoft
*/
#if COMPILER == COMPILER_MICROSOFT
#  define snprintf _snprintf
#  define vsnprintf _vsnprintf
#  define finite(X) _finite(X)
#endif
#include "GDBCTypedef.h"
#include "GDBCVersion.h"


#endif // __GDBC_GDBCPREREQUISITES_H__