#if !defined(QUERYRESULTMYSQL_H)
#define QUERYRESULTMYSQL_H

#include "GDBCPrerequisities.h"
#include "Database/GDBCQueryResult.h"

#if GDBC_PLATFORM == GDBC_PLATFORM_WIN32
#include <winsock2.h>
#include <mysql.h>
#else
#include "mysql.h"
#endif

class QueryResultMysql : public QueryResult
{
    public:
        QueryResultMysql(MYSQL_RES *result, MYSQL_FIELD *fields, GDBC::uint64 rowCount, GDBC::uint32 fieldCount);

        ~QueryResultMysql();

        bool NextRow();

    private:
        enum Field::DataTypes ConvertNativeType(enum_field_types mysqlType) const;
        void EndQuery();

        MYSQL_RES *mResult;
};
#endif
