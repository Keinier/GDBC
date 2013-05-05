/******************************************************************************
*                                                                             *
*       File Name: GDBCPlatform.h                                             *
*       Description: Define all Platform's directive for post configurations. *
*                                                                             *
*       Author: Keinier Caboverde Ramírez. <keinier@gmail.com>                *
*       Date: 03/05/2013 08:51 a.m.                                           *
*       File's Version: 1.3.1                                                 *
*                                                                             *
******************************************************************************/

#ifndef __GDBC_GDBCPLATFORM_H__
#define __GDBC_GDBCPLATFORM_H__


/* Constantes de las posibles plataformas y compiladores soportados 
   por la libreria dinámica */

#define GDBC_PLATFORM_WIN32		0x01
#define GDBC_PLATFORM_LINUX		0x02
#define GDBC_PLATFORM_APPLE		0x03
#define GDBC_PLATFORM_SYMBIAN	0x04
#define GDBC_PLATFORM_IPHONE	0x05

#define GDBC_COMPILER_MSVC		0x01
#define GDBC_COMPILER_GNUC		0x02
#define GDBC_COMPILER_BORL		0x03
#define GDBC_COMPILER_WINSCW	0x04
#define GDBC_COMPILER_GCCE		0x05

#define GDBC_ENDIAN_LITTLE		0x01
#define GDBC_ENDIAN_BIG			0x02

#define GDBC_ARCHITECTURE_32	0x01
#define GDBC_ARCHITECTURE_64	0x02

/* Determinar la version y tipo de compilador */

#if defined( __GCCE__ )				// Si el compilador es GCC
#   define GDBC_COMPILER GDBC_COMPILER_GCCE
#   define GDBC_COMP_VER _MSC_VER
#elif defined( __WINSCW__ )
#   define GDBC_COMPILER GDBC_COMPILER_WINSCW
#   define GDBC_COMP_VER _MSC_VER
#elif defined( _MSC_VER )
#   define GDBC_COMPILER GDBC_COMPILER_MSVC
#	pragma warning(disable:4244)
#	pragma warning(disable:4996)
#   define GDBC_COMP_VER _MSC_VER
#elif defined( __GNUC__ )
#   define GDBC_COMPILER GDBC_COMPILER_GNUC
#   define GDBC_COMP_VER (((__GNUC__)*100) + (__GNUC_MINOR__*10) + __GNUC_PATCHLEVEL__)
#elif defined( __BORLANDC__ )
#   define GDBC_COMPILER GDBC_COMPILER_BORL
#   define GDBC_COMP_VER __BCPLUSPLUS__
#   define __FUNCTION__ __FUNC__ 
#else
#   pragma error "No known compiler. Abort! Abort!"
#endif

/* Determinar si es mejor usar __forceinline o __inline automaticamente */

#if GDBC_COMPILER == GDBC_COMPILER_MSVC
#   if GDBC_COMP_VER >= 1200
#       define FORCEINLINE __forceinline
#   endif
#elif defined(__MINGW32__)
#   if !defined(FORCEINLINE)
#       define FORCEINLINE __inline
#   endif
#else
#   define FORCEINLINE __inline
#endif


/* Encontrar la plataforma actual */

#if defined( __SYMBIAN32__ ) 
#   define GDBC_PLATFORM GDBC_PLATFORM_SYMBIAN
#elif defined( __WIN32__ ) || defined( _WIN32 ) || defined(WIN32)
#   define GDBC_PLATFORM GDBC_PLATFORM_WIN32
#elif defined( __APPLE_CC__)
#   if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000
#       define GDBC_PLATFORM GDBC_PLATFORM_IPHONE
#   else
#       define GDBC_PLATFORM GDBC_PLATFORM_APPLE
#   endif
#else
#   define GDBC_PLATFORM GDBC_PLATFORM_LINUX
#endif

/* Detectar la arquitectura del microposesador en el sistema actual */
#if defined(__x86_64__) || defined(_M_X64) || defined(__powerpc64__) || defined(__alpha__) || defined(__ia64__) || defined(__s390__) || defined(__s390x__)
#   define GDBC_ARCH_TYPE GDBC_ARCHITECTURE_64
#else
#   define GDBC_ARCH_TYPE GDBC_ARCHITECTURE_32
#endif

// Macros para generar un mensaje de Advertencia
#define GDBC_QUOTE_INPLACE(x) # x
#define GDBC_QUOTE(x) GDBC_QUOTE_INPLACE(x)
#define GDBC_WARN( x )  message( __FILE__ "(" QUOTE( __LINE__ ) ") : " x "\n" )

//----------------------------------------------------------------------------
// Configuracion para plataforma WINDOWS
#if GDBC_PLATFORM == GDBC_PLATFORM_WIN32

// Si no se ha especificado si el compilado no es cliente, los simbolos se 
// importaran de lo contrario se exportaran
#	if defined( GDBC_STATIC_LIB )
#   	define _GDBCAPIExport
#   	define _GDBCAPIPrivate
#   else
#   	if defined(GDBC_NONCLIENT_BUILD)
#       	define _GDBCAPIExport __declspec( dllexport )
#   	else
#           if defined( __MINGW32__ )
#               define _GDBCAPIExport
#           else
#       	    define _GDBCAPIExport __declspec( dllimport )
#           endif
#   	endif
#   	define _GDBCAPIPrivate
#	endif

// determina el modo de compilacion
#   if defined(_DEBUG) || defined(DEBUG)
#       define GDBC_DEBUG_MODE 1
#   else
#       define GDBC_DEBUG_MODE 0
#   endif


#if defined( __MINGW32__ ) && !defined(_STLPORT_VERSION)
#   include<_mingw.h>
#   if defined(__MINGW32_TOOLBOX_UNICODE__) || GDBC_COMP_VER > 345
#	    define GDBC_UNICODE_SUPPORT 1
#   else
#       define GDBC_UNICODE_SUPPORT 0
#   endif
#else
#	define GDBC_UNICODE_SUPPORT 1
#endif

#endif


// Configuración Linux/Apple/Symbian
#if GDBC_PLATFORM == GDBC_PLATFORM_LINUX || GDBC_PLATFORM == GDBC_PLATFORM_APPLE || GDBC_PLATFORM == GDBC_PLATFORM_IPHONE 

// Habilitar la visibilidad de simbolos
#   if defined( GDBC_GCC_VISIBILITY )
#       define _GDBCAPIExport  __attribute__ ((visibility("default")))
#       define _GDBCAPIPrivate __attribute__ ((visibility("hidden")))
#   else
#       define _GDBCAPIExport
#       define _GDBCAPIPrivate
#   endif
#   define stricmp strcasecmp


#   ifdef DEBUG
#       define GDBC_DEBUG_MODE 1
#   else
#       define GDBC_DEBUG_MODE 0
#   endif

#define GDBC_UNICODE_SUPPORT 1
#endif

// Configuracion de Endian

#ifdef GDBC_CONFIG_BIG_ENDIAN
#    define GDBC_ENDIAN GDBC_ENDIAN_BIG
#else
#    define GDBC_ENDIAN GDBC_ENDIAN_LITTLE
#endif

#if GDBC_PLATFORM != GDBC_PLATFORM_WIN32
/*
**  Macros para operacion primitivas con bit
*/
#	define LOBYTE(w)       ((BYTE) (w))
#	define HIBYTE(w)       ((BYTE) ((0x00FF & (WORD) ((w) >>  8))))
#	define LOWORD(d)       ((WORD) (d))
#	define HIWORD(d)       ((WORD) ((0xFFFF & (DWORD)((d) >> 16))))
#	define MAKEWORD(h,l)   (((WORD)  ((WORD)  (h) <<  8)) | (WORD) ((BYTE) (l)))
#	define MAKEDWORD(h,l)  (((DWORD) ((DWORD) (h) << 16)) | (DWORD)((WORD) (l)))
#endif


#endif // __GDBC_GDBCPLATFORM_H__