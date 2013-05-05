#ifndef __GDBC_GDBCDATABASEMYSQL_H__
#define __GDBC_GDBCDATABASEMYSQL_H__

#include "GDBCPrerequisities.h"
#include "GDBCDatabase.h"

enum MysqlTypes { GDBC_TYPE_DECIMAL, GDBC_TYPE_TINY,
                        GDBC_TYPE_SHORT,  GDBC_TYPE_LONG,
                        GDBC_TYPE_FLOAT,  GDBC_TYPE_DOUBLE,
                        GDBC_TYPE_NULL,   GDBC_TYPE_TIMESTAMP,
                        GDBC_TYPE_LONGLONG,GDBC_TYPE_INT24,
                        GDBC_TYPE_DATE,   GDBC_TYPE_TIME,
                        GDBC_TYPE_DATETIME, GDBC_TYPE_YEAR,
                        GDBC_TYPE_NEWDATE, GDBC_TYPE_VARCHAR,
                        GDBC_TYPE_BIT,
                        GDBC_TYPE_NEWDECIMAL=246,
                        GDBC_TYPE_ENUM=247,
                        GDBC_TYPE_SET=248,
                        GDBC_TYPE_TINY_BLOB=249,
                        GDBC_TYPE_MEDIUM_BLOB=250,
                        GDBC_TYPE_LONG_BLOB=251,
                        GDBC_TYPE_BLOB=252,
                        GDBC_TYPE_VAR_STRING=253,
                        GDBC_TYPE_STRING=254,
                        GDBC_TYPE_GEOMETRY=255,
                        GDBC_MAX_NO_FIELD_TYPES /* Should always be last */
};

//MySQL prepared statement class
class _GDBCAPIExport MySqlPreparedStatement : public SqlPreparedStatement
{
public:
    MySqlPreparedStatement(const std::string& fmt, SqlConnection& conn, void * mysql);
    ~MySqlPreparedStatement();

    //prepare statement
    virtual bool prepare();

    //bind input parameters
    virtual void bind(const SqlStmtParameters& holder);

    //execute DML statement
    virtual bool execute();

protected:
    //bind parameters
    void addParam(int nIndex, const SqlStmtFieldData& data);

    static MysqlTypes ToMySQLType( const SqlStmtFieldData &data, bool &bUnsigned );

private:
    void RemoveBinds();

    /*MYSQL*/void * m_pMySQLConn;
    /*MYSQL_STMT*/void* m_stmt;
    /*MYSQL_BIND*/void* m_pInputArgs;
    /*MYSQL_BIND*/void* m_pResult;
    /*MYSQL_RES*/ void* m_pResultMetadata;
};

class _GDBCAPIExport MySQLConnection : public SqlConnection
{
    public:
        MySQLConnection(Database& db) : SqlConnection(db), mMysql(NULL) {}
        ~MySQLConnection();

        //! Initializes Mysql and connects to a server.
        /*! infoString should be formated like hostname;username;password;database. */
        bool Initialize(const char *infoString);

        QueryResult* Query(const char *sql);
        QueryNamedResult* QueryNamed(const char *sql);
        bool Execute(const char *sql);

        unsigned long escape_string(char *to, const char *from, unsigned long length);

        bool BeginTransaction();
        bool CommitTransaction();
        bool RollbackTransaction();

    protected:
        SqlPreparedStatement * CreateStatement(const std::string& fmt);

    private:
        bool _TransactionCmd(const char *sql);
        bool _Query(const char *sql, void **pResult, void **pFields, GDBC::uint64* pRowCount, GDBC::uint32* pFieldCount);

        /*MYSQL*/void *mMysql;
};

class _GDBCAPIExport DatabaseMysql : public Database
{
    //friend class MaNGOS::OperatorNew<DatabaseMysql>;

    public:
        DatabaseMysql();
        ~DatabaseMysql();

        // must be call before first query in thread
        void ThreadStart();
        // must be call before finish thread run
        void ThreadEnd();

    protected:
        virtual SqlConnection * CreateConnection();

    private:
        static size_t db_count;
};

#endif
