#ifndef __GDBC_GDBCQUERYRESULT_H__
#define __GDBC_GDBCQUERYRESULT_H__

#include "GDBCPrerequisities.h"
#include "GDBCField.h"

class _GDBCAPIExport QueryResult
{
    public:
        QueryResult(GDBC::uint64 rowCount, GDBC::uint32 fieldCount)
            : mFieldCount(fieldCount), mRowCount(rowCount) {}

        virtual ~QueryResult() {}

        virtual bool NextRow() = 0;

        Field *Fetch() const { return mCurrentRow; }

        const Field & operator [] (int index) const { return mCurrentRow[index]; }

        GDBC::uint32 GetFieldCount() const { return mFieldCount; }
        GDBC::uint64 GetRowCount() const { return mRowCount; }

    protected:
        Field *mCurrentRow;
        GDBC::uint32 mFieldCount;
        GDBC::uint64 mRowCount;
};

template class _GDBCAPIExport std::allocator<std::string>;
template class _GDBCAPIExport std::vector<std::string>;

typedef std::vector<std::string> QueryFieldNames;

class _GDBCAPIExport QueryNamedResult
{
    public:
        explicit QueryNamedResult(QueryResult* query, QueryFieldNames const& names) : mQuery(query), mFieldNames(names) {}
        ~QueryNamedResult() { delete mQuery; }

        // compatible interface with QueryResult
        bool NextRow() { return mQuery->NextRow(); }
        Field *Fetch() const { return mQuery->Fetch(); }
        GDBC::uint32 GetFieldCount() const { return mQuery->GetFieldCount(); }
        GDBC::uint64 GetRowCount() const { return mQuery->GetRowCount(); }
        Field const& operator[] (int index) const { return (*mQuery)[index]; }

        // named access
        Field const& operator[] (const std::string &name) const { return mQuery->Fetch()[GetField_idx(name)]; }
        QueryFieldNames const& GetFieldNames() const { return mFieldNames; }

        GDBC::uint32 GetField_idx(const std::string &name) const
        {
            for(size_t idx = 0; idx < mFieldNames.size(); ++idx)
            {
                if(mFieldNames[idx] == name)
                    return idx;
            }
            assert(false && "unknown field name");
            return GDBC::uint32(-1);
        }

    protected:
        QueryResult *mQuery;
        QueryFieldNames mFieldNames;
};

#endif // __GDBC_GDBCQUERYRESULT_H__
