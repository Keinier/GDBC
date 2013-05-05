#ifndef __GDBC_GDBCSQLSTORAGE_H__
#define __GDBC_GDBCSQLSTORAGE_H__

#include "GDBCPrerequisities.h"
#include "Database/GDBCField.h"
#include "Database/GDBCQueryResult.h"
#include "Database/GDBCSqlOperations.h"
#include "Database/GDBCDatabase.h"

class SQLStorage
{
    template<class T>
    friend struct SQLStorageLoaderBase;

    public:

        SQLStorage(const char* fmt, const char * _entry_field, const char * sqlname)
        {
            src_format = fmt;
            dst_format = fmt;
            init(_entry_field, sqlname);
        }

        SQLStorage(const char* src_fmt, const char* dst_fmt, const char * _entry_field, const char * sqlname)
        {
            src_format = src_fmt;
            dst_format = dst_fmt;
            init(_entry_field, sqlname);
        }

        ~SQLStorage()
        {
            Free();
        }

        template<class T>
            T const* LookupEntry(GDBC::uint32 id) const
        {
            if( id == 0 )
                return NULL;
            if(id >= MaxEntry)
                return NULL;
            return reinterpret_cast<T const*>(pIndex[id]);
        }

        GDBC::uint32 RecordCount;
        GDBC::uint32 MaxEntry;
        GDBC::uint32 iNumFields;

        char const* GetTableName() const { return table; }

        void Load();
        void Free();

        void EraseEntry(GDBC::uint32 id);
    private:
        void init(const char * _entry_field, const char * sqlname)
        {
            entry_field = _entry_field;
            table=sqlname;
            data=NULL;
            pIndex=NULL;
            iNumFields = strlen(src_format);
            MaxEntry = 0;
        }

        char** pIndex;

        char *data;
        const char *src_format;
        const char *dst_format;
        const char *table;
        const char *entry_field;
        //bool HasString;
};

template <class T>
struct SQLStorageLoaderBase
{
    public:
        void Load(SQLStorage &storage, bool error_at_empty = true);

        template<class S, class D>
            void convert(GDBC::uint32 field_pos, S src, D &dst);

        template<class S>
            void convert_to_str(GDBC::uint32 field_pos, S src, char * & dst);

        template<class D>
            void convert_from_str(GDBC::uint32 field_pos, char const* src, D& dst);
        void convert_str_to_str(GDBC::uint32 field_pos, char const* src, char *&dst);

        // trap, no body
        template<class D>
            void convert_from_str(GDBC::uint32 field_pos, char* src, D& dst);

        void convert_str_to_str(GDBC::uint32 field_pos, char* src, char *&dst);
    private:
        template<class V>
            void storeValue(V value, SQLStorage &store, char *p, GDBC::uint32 x, GDBC::uint32 &offset);
        void storeValue(char const* value, SQLStorage &store, char *p, GDBC::uint32 x, GDBC::uint32 &offset);

        // trap, no body
        void storeValue(char * value, SQLStorage &store, char *p, GDBC::uint32 x, GDBC::uint32 &offset);
};

struct SQLStorageLoader : public SQLStorageLoaderBase<SQLStorageLoader>
{
};

#endif
