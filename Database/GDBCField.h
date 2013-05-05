#ifndef __GDBC_GDBCFIELD_H__
#define __GDBC_GDBCFIELD_H__

#include "GDBCPrerequisities.h"

class Field
{
    public:

        enum DataTypes
        {
            DB_TYPE_UNKNOWN = 0x00,
            DB_TYPE_STRING  = 0x01,
            DB_TYPE_INTEGER = 0x02,
            DB_TYPE_FLOAT   = 0x03,
            DB_TYPE_BOOL    = 0x04
        };

        Field() : mValue(NULL), mType(DB_TYPE_UNKNOWN) {}
        Field(const char* value, enum DataTypes type) : mValue(value), mType(type) {}

        ~Field() {}

        enum DataTypes GetType() const { return mType; }
        bool IsNULL() const { return mValue == NULL; }

        const char *GetString() const { return mValue; }
        std::string GetCppString() const
        {
            return mValue ? mValue : "";                    // std::string s = 0 have undefine result in C++
        }

        float GetFloat() const { return mValue ? static_cast<float>(atof(mValue)) : 0.0f; }
        bool GetBool() const { return mValue ? atoi(mValue) > 0 : false; }
        GDBC::int32 GetInt32() const { return mValue ? static_cast<GDBC::int32>(atol(mValue)) : GDBC::int32(0); }
        GDBC::uint8 GetUInt8() const { return mValue ? static_cast<GDBC::uint8>(atol(mValue)) : GDBC::uint8(0); }
        GDBC::uint16 GetUInt16() const { return mValue ? static_cast<GDBC::uint16>(atol(mValue)) : GDBC::uint16(0); }
        GDBC::int16 GetInt16() const { return mValue ? static_cast<GDBC::int16>(atol(mValue)) : GDBC::int16(0); }
        GDBC::uint32 GetUInt32() const { return mValue ? static_cast<GDBC::uint32>(atol(mValue)) : GDBC::uint32(0); }
        GDBC::uint64 GetUInt64() const
        {
            GDBC::uint64 value = 0;
            if(!mValue || sscanf(mValue,"%llu"/*UI64FMTD*/,&value) == -1)
                return 0;

            return value;
        }

        void SetType(enum DataTypes type) { mType = type; }
        //no need for memory allocations to store resultset field strings
        //all we need is to cache pointers returned by different DBMS APIs
        void SetValue(const char* value) { mValue = value; };

    private:
        Field(Field const&);
        Field& operator=(Field const&);

        const char* mValue;
        enum DataTypes mType;
};

#endif // __GDBC_GDBCFIELD_H__
