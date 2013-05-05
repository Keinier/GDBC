/******************************************************************************
*                                                                             *
*       File Name: GDBCTypedef.h                                              *
*       Description: Define the basic data types.                             *
*                                                                             *
*       Author: Keinier Caboverde Ramírez. <keinier@gmail.com>                *
*       Date: 03/05/2013 08:51 a.m.                                           *
*       File's Version: 1.3.1                                                 *
*                                                                             *
******************************************************************************/
#ifndef __GDBC_GDBCTYPEDEF_H__
#define __GDBC_GDBCTYPEDEF_H__

#include <string>
#include <vector>
#include <sstream>

namespace GDBC
{
	typedef unsigned char      uint8;
	typedef signed char		   int8;
	typedef unsigned short	   uint16;
	typedef signed short	   int16;
	typedef unsigned int	   uint32;
	typedef signed int		   int32;
	typedef float			   float32;
	typedef long long	       int64;
	typedef unsigned long long uint64;
	typedef uint32             size_type;
	typedef time_t             time_type;
	typedef unsigned int	   uint;
	typedef signed int		   sint;

#ifndef _WINDEF_
	typedef unsigned long       DWORD;
	typedef int                 BOOL;
	typedef unsigned char       BYTE;
	typedef unsigned short      WORD;
	typedef void*				LPVOID;
	typedef void				VOID;
#endif
		
#ifdef DOUBLE_PRECISION		
	typedef double			                     Real;
#else		
	typedef float			                     Real;
#endif
	typedef double                               DOUBLE;
	typedef float                                FLOAT;		
	typedef char                                 CHAR;
	typedef wchar_t                              WCHAR;
	typedef std::basic_string<wchar_t>           _WStringBase;
	typedef std::basic_string<char>              _StringBase;
	typedef std::basic_stringstream<wchar_t>     _WStringStreamBase;
	typedef std::basic_stringstream<char>        _StringStreamBase;
	typedef _WStringBase                         WString;
	typedef _WStringStreamBase                   WStringStream;
	typedef std::vector<WString>                 WTokens;
	typedef _StringBase                          String;
	typedef _StringStreamBase                    StringStream;
	typedef std::vector<String>                  Tokens;
}
#ifndef _WINDEF_
#	define FALSE           0           /* FALSE boolean data value   */
#	define TRUE            !FALSE      /* TRUE boolean data value    */
#endif

		
/*
**  A variety of macros used to test/flip bits within words of data.
*/
/* Flip the specified bit to 0 */
#	define BITCLEAR(field, bit)  ((field) & ~(bit))

/* Flip the specified bit to 1 */
#	define BITSET(field, bit)    ((field) | (bit))

/* Flip the specified bit to its inverse state */
#	define BITTOG(field, bit)    ((field) ^ (bit))

/* Create a string of 0 bits from n to m */
#	define BITSON(n, m)          (~(~0 << (m) << 0) & (~0 << (n)))

/* Create a string of 1 bits from n to m */
#	define BITSOFF(n, m)         (~(~0 << (m) << 1) & (~0 << (n)))


/* Structure copy macro */
#	if defined(__STDC__)
#		define assignstruct(a, b)  ((a) = (b))
#	else
#		define assignstruct(a, b)  (memcpy((char*)&(a), (char *)&(b),sizeof(a)))
#	endif

#	if !defined(offsetof)
#		define offsetof(s, m)  ((size_t)&(((s *)0)->m))
#	endif

#	if !defined(num_max)
#		define num_max(a,b)        (((a) > (b)) ? (a) : (b))
#	endif

#	if !defined(num_min)
#		define num_min(a,b)        (((a) < (b)) ? (a) : (b))
#	endif

#define GDBC_CHAR_MAX 0x7F
#define GDBC_CHAR_MIN -(GDBC_CHAR_MAX)-1
#define GDBC_OCTET_MAX 0xFF
#define GDBC_INT16_MAX 0x7FFF
#define GDBC_INT16_MIN -(GDBC_INT16_MAX)-1
#define GDBC_UINT16_MAX 0xFFFF
#define GDBC_WCHAR_MAX GDBC_UINT16_MAX
#define GDBC_INT32_MAX 0x7FFFFFFF
#define GDBC_INT32_MIN -(GDBC_INT32_MAX)-1
#define GDBC_UINT32_MAX 0xFFFFFFFF
#define GDBC_INT64_MAX GDBC_INT64_LITERAL(0x7FFFFFFFFFFFFFFF)
#define GDBC_INT64_MIN -(GDBC_INT64_MAX)-1

#endif // __GDBC_GDBCTYPEDEF_H__