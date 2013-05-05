#ifndef __GDBC_GDBCBYTECONVERTER_H__
#define __GDBC_GDBCBYTECONVERTER_H__

/** ByteConverter reverse your byte order.  This is use
    for cross platform where they have different endians.
 */
#include "GDBCPrerequisities.h"


namespace ByteConverter
{
    template<size_t T>
    inline void convert(char *val)
    {
        std::swap(*val, *(val + T - 1));
        convert<T - 2>(val + 1);
    }

    template<> inline void convert<0>(char *) {}
    template<> inline void convert<1>(char *) {}            // ignore central byte

    template<typename T>
    inline void apply(T *val)
    {
        convert<sizeof(T)>((char *)(val));
    }
}

#if GDBC_ENDIAN == GDBC_ENDIAN_BIG
template<typename T> inline void EndianConvert(T& val) { ByteConverter::apply<T>(&val); }
template<typename T> inline void EndianConvertReverse(T&) { }
#else
template<typename T> inline void EndianConvert(T&) { }
template<typename T> inline void EndianConvertReverse(T& val) { ByteConverter::apply<T>(&val); }
#endif

template<typename T> void EndianConvert(T*);         // will generate link error
template<typename T> void EndianConvertReverse(T*);  // will generate link error

inline void EndianConvert(GDBC::uint8&) { }
inline void EndianConvert(GDBC::int8&)  { }
inline void EndianConvertReverse(GDBC::uint8&) { }
inline void EndianConvertReverse( GDBC::int8&) { }

#endif // __GDBC_GDBCBYTECONVERTER_H__