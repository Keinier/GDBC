#ifndef __GDBC_GDBCDBCSTORE_H__
#define __GDBC_GDBCDBCSTORE_H__
#include "GDBCPrerequisities.h"
#include "GDBCDBCFileLoader.h"

template<class T>
class DBCStorage
{
    typedef std::list<char*> StringPoolList;
    public:
        explicit DBCStorage(const char *f) : nCount(0), fieldCount(0), fmt(f), indexTable(NULL), m_dataTable(NULL) { }
        ~DBCStorage() { Clear(); }

        T const* LookupEntry(GDBC::uint32 id) const { return (id>=nCount)?NULL:indexTable[id]; }
        GDBC::uint32  GetNumRows() const { return nCount; }
        char const* GetFormat() const { return fmt; }
        GDBC::uint32 GetFieldCount() const { return fieldCount; }

        bool Load(char const* fn)
        {
            DBCFileLoader dbc;
            // Check if load was sucessful, only then continue
            if(!dbc.Load(fn, fmt))
                return false;

            fieldCount = dbc.GetCols();

            // load raw non-string data
            m_dataTable = (T*)dbc.AutoProduceData(fmt,nCount,(char**&)indexTable);

            // load strings from dbc data
            m_stringPoolList.push_back(dbc.AutoProduceStrings(fmt,(char*)m_dataTable));

            // error in dbc file at loading if NULL
            return indexTable!=NULL;
        }

        bool LoadStringsFrom(char const* fn)
        {
            // DBC must be already loaded using Load
            if(!indexTable)
                return false;

            DBCFileLoader dbc;
            // Check if load was successful, only then continue
            if(!dbc.Load(fn, fmt))
                return false;

            // load strings from another locale dbc data
            m_stringPoolList.push_back(dbc.AutoProduceStrings(fmt,(char*)m_dataTable));

            return true;
        }

        void Clear()
        {
            if (!indexTable)
                return;

            delete[] ((char*)indexTable);
            indexTable = NULL;
            delete[] ((char*)m_dataTable);
            m_dataTable = NULL;

            while(!m_stringPoolList.empty())
            {
                delete[] m_stringPoolList.front();
                m_stringPoolList.pop_front();
            }
            nCount = 0;
        }

        void EraseEntry(GDBC::uint32 id) { indexTable[id] = NULL; }

    private:
        GDBC::uint32 nCount;
        GDBC::uint32 fieldCount;
        char const* fmt;
        T** indexTable;
        T* m_dataTable;
        StringPoolList m_stringPoolList;
};

#endif
