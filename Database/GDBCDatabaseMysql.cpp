

#include "Database/GDBCDatabaseMysql.h"
#include "Database/GDBCQueryResultMysql.h"
#include "GDBCThreading.h"


#include "GDBCUtilities.h"
#include "ace/Thread_Mutex.h"
#include "ace/Guard_T.h"

#if GDBC_PLATFORM == GDBC_PLATFORM_WIN32
#include <winsock2.h>
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

size_t DatabaseMysql::db_count = 0;

void DatabaseMysql::ThreadStart()
{
    mysql_thread_init();
}

void DatabaseMysql::ThreadEnd()
{
    mysql_thread_end();
}

DatabaseMysql::DatabaseMysql()
{
    // before first connection
    if( db_count++ == 0 )
    {
        // Mysql Library Init
        mysql_library_init(-1, NULL, NULL);

        if (!mysql_thread_safe())
        {
            std::cout << "FATAL ERROR: Used MySQL library isn't thread-safe."; //SYSTEM_LOG("FATAL ERROR: Used MySQL library isn't thread-safe.");
            exit(1);
        }
    }
}

DatabaseMysql::~DatabaseMysql()
{
    StopServer();

    //Free Mysql library pointers for last ~DB
    if(--db_count == 0)
        mysql_library_end();
}

SqlConnection * DatabaseMysql::CreateConnection()
{
    return new MySQLConnection(*this);
}

MySQLConnection::~MySQLConnection()
{
    FreePreparedStatements();
    mysql_close((MYSQL*)mMysql);
}

bool MySQLConnection::Initialize(const char *infoString)
{
    MYSQL * mysqlInit = mysql_init(NULL);
    if (!mysqlInit)
    {
        std::cout << "Could not initialize Mysql connection"<<std::endl; //SYSTEM_LOG( "Could not initialize Mysql connection" );
        return false;
    }

    GDBC::Tokens tokens = StrSplit(infoString, ";");

    GDBC::Tokens::iterator iter;

    GDBC::String host, port_or_socket, user, password, database;
    int port;
    char const* unix_socket;

    iter = tokens.begin();

    if(iter != tokens.end())
        host = *iter++;
    if(iter != tokens.end())
        port_or_socket = *iter++;
    if(iter != tokens.end())
        user = *iter++;
    if(iter != tokens.end())
        password = *iter++;
    if(iter != tokens.end())
        database = *iter++;

    mysql_options(mysqlInit,MYSQL_SET_CHARSET_NAME,"utf8");
#if GDBC_PLATFORM == GDBC_PLATFORM_WIN32
    if(host==".")                                           // named pipe use option (Windows)
    {
        unsigned int opt = MYSQL_PROTOCOL_PIPE;
        mysql_options(mysqlInit,MYSQL_OPT_PROTOCOL,(char const*)&opt);
        port = 0;
        unix_socket = 0;
    }
    else                                                    // generic case
    {
        port = atoi(port_or_socket.c_str());
        unix_socket = 0;
    }
#else
    if(host==".")                                           // socket use option (Unix/Linux)
    {
        unsigned int opt = MYSQL_PROTOCOL_SOCKET;
        mysql_options(mysqlInit,MYSQL_OPT_PROTOCOL,(char const*)&opt);
        host = "localhost";
        port = 0;
        unix_socket = port_or_socket.c_str();
    }
    else                                                    // generic case
    {
        port = atoi(port_or_socket.c_str());
        unix_socket = 0;
    }
#endif

    mMysql = mysql_real_connect(mysqlInit, host.c_str(), user.c_str(),
        password.c_str(), database.c_str(), port, unix_socket, 0);

    if (!mMysql)
    {
        std::stringstream ss;
        ss << "Could not connect to MySQL database at " << host.c_str() << ": " << mysql_error(mysqlInit)<<std::endl;
        std::cout << ss.str(); //SYSTEM_LOG(ss.str());
        mysql_close(mysqlInit);
        return false;
    }
    
    std::stringstream ss;
    ss << "Connected to MySQL database " << user.c_str() << "@" <<host.c_str() << ":" <<port_or_socket.c_str() << "/" <<database.c_str()<<std::endl;
    ss <<"MySQL client library: " << mysql_get_client_info()<<std::endl;  
    ss<<"MySQL server ver: " << mysql_get_server_info((MYSQL*) mMysql);
	std::cout << ss.str() <<std::endl;

    /*----------SET AUTOCOMMIT ON---------*/
    // It seems mysql 5.0.x have enabled this feature
    // by default. In crash case you can lose data!!!
    // So better to turn this off
    // ---
    // This is wrong since mangos use transactions,
    // autocommit is turned of during it.
    // Setting it to on makes atomic updates work
    // ---
    // LEAVE 'AUTOCOMMIT' MODE ALWAYS ENABLED!!!
    // W/O IT EVEN 'SELECT' QUERIES WOULD REQUIRE TO BE WRAPPED INTO 'START TRANSACTION'<>'COMMIT' CLAUSES!!!
    if (!mysql_autocommit((MYSQL*)mMysql, 1))
    {
        printf("AUTOCOMMIT SUCCESSFULLY SET TO 1\n");
    }
    else{
        printf("AUTOCOMMIT NOT SET TO 1\n");
    }
    /*-------------------------------------*/

    // set connection properties to UTF8 to properly handle locales for different
    // server configs - core sends data in UTF8, so MySQL must expect UTF8 too
    Execute("SET NAMES `utf8`");
    Execute("SET CHARACTER SET `utf8`");

    return true;
}

bool MySQLConnection::_Query(const char *sql, void **pResult, void **pFields, GDBC::uint64* pRowCount, GDBC::uint32* pFieldCount)
{
    if (!mMysql)
        return 0;

    //GDBC::uint32 _s = WorldTimer::getMSTime();

    if(mysql_query((MYSQL*)mMysql, sql))
    {
        std::stringstream ss;
        ss<< "SQL: " << sql;
        std::cout << ss.str(); //SYSTEM_LOG(ss.str());
        ss.str("");
        ss << "query ERROR: " << mysql_error((MYSQL*)mMysql);
        std::cout << ss.str()<<std::endl;//SYSTEM_LOG(ss.str());
        return false;
    }
    else
    {
        std::stringstream ss;
        ss <</* "[" << WorldTimer::getMSTimeDiff(_s,WorldTimer::getMSTime()) << " ms] SQL: "*/ "SQL: " << sql;
        std::cout << ss.str();//SYSTEM_LOG(ss.str());
    }

    *pResult = mysql_store_result((MYSQL*)mMysql);
    *pRowCount = mysql_affected_rows((MYSQL*)mMysql);
    *pFieldCount = mysql_field_count((MYSQL*)mMysql);

    if (!*pResult )
        return false;

    if (!*pRowCount)
    {
        mysql_free_result((MYSQL_RES*)*pResult);
        return false;
    }

    *pFields = mysql_fetch_fields((MYSQL_RES*)*pResult);
    return true;
}

QueryResult* MySQLConnection::Query(const char *sql)
{
    MYSQL_RES *result = NULL;
    MYSQL_FIELD *fields = NULL;
    GDBC::uint64 rowCount = 0;
    GDBC::uint32 fieldCount = 0;

    if(!_Query(sql,(void**)&result,(void**)&fields,&rowCount,&fieldCount))
        return NULL;

    QueryResultMysql *queryResult = new QueryResultMysql(result, fields, rowCount, fieldCount);

    queryResult->NextRow();
    return queryResult;
}

QueryNamedResult* MySQLConnection::QueryNamed(const char *sql)
{
    MYSQL_RES *result = NULL;
    MYSQL_FIELD *fields = NULL;
    GDBC::uint64 rowCount = 0;
    GDBC::uint32 fieldCount = 0;

    if(!_Query(sql,(void**)&result,(void**)&fields,&rowCount,&fieldCount))
        return NULL;

    QueryFieldNames names(fieldCount);
    for (GDBC::uint32 i = 0; i < fieldCount; i++)
        names[i] = fields[i].name;

    QueryResultMysql *queryResult = new QueryResultMysql(result, fields, rowCount, fieldCount);

    queryResult->NextRow();
    return new QueryNamedResult(queryResult,names);
}

bool MySQLConnection::Execute(const char* sql)
{
    if (!mMysql)
        return false;

    {
       // GDBC::uint32 _s = WorldTimer::getMSTime();

        if(mysql_query((MYSQL*)mMysql, sql))
        {
                std::stringstream ss;
                ss<< "SQL: " << sql;
                std::cout << ss.str(); // SYSTEM_LOG(ss.str());
                ss.str("");
                ss << "SQL ERROR: " << mysql_error((MYSQL*)mMysql);
                std::cout << ss.str(); //SYSTEM_LOG(ss.str());
                return false;
        }
        else
        {
                std::stringstream ss;
                ss << " SQL: " << sql;
                std::cout << ss.str();// SYSTEM_LOG(ss.str());
        }
        // end guarded block
    }

    return true;
}

bool MySQLConnection::_TransactionCmd(const char *sql)
{
    if (mysql_query((MYSQL*)mMysql, sql))
    {
        std::stringstream ss;
        ss<< "SQL: " << sql;
        std::cout << ss.str();//SYSTEM_LOG(ss.str());
        ss.str("");
        ss << "SQL ERROR: " << mysql_error((MYSQL*)mMysql);
        std::cout << ss.str();//SYSTEM_LOG(ss.str());
        return false;
    }
    else
    {
        std::stringstream ss;
        ss<< "SQL: " << sql;
        std::cout << ss.str();//SYSTEM_LOG(ss.str());
    }
    return true;
}

bool MySQLConnection::BeginTransaction()
{
    return _TransactionCmd("START TRANSACTION");
}

bool MySQLConnection::CommitTransaction()
{
    return _TransactionCmd("COMMIT");
}

bool MySQLConnection::RollbackTransaction()
{
    return _TransactionCmd("ROLLBACK");
}

unsigned long MySQLConnection::escape_string(char *to, const char *from, unsigned long length)
{
    if (!mMysql || !to || !from || !length)
        return 0;

    return(mysql_real_escape_string((MYSQL*)mMysql, to, from, length));
}

//////////////////////////////////////////////////////////////////////////
SqlPreparedStatement * MySQLConnection::CreateStatement( const std::string& fmt )
{
    return new MySqlPreparedStatement(fmt, *this, (MYSQL*)mMysql);
}


//////////////////////////////////////////////////////////////////////////
MySqlPreparedStatement::MySqlPreparedStatement( const std::string& fmt, SqlConnection& conn, void * mysql ) : SqlPreparedStatement(fmt, conn),
    m_pMySQLConn(mysql), m_stmt(NULL), m_pInputArgs(NULL), m_pResult(NULL), m_pResultMetadata(NULL)
{
}

MySqlPreparedStatement::~MySqlPreparedStatement()
{
    RemoveBinds();
}

bool MySqlPreparedStatement::prepare()
{
    if(isPrepared())
        return true;

    //remove old binds
    RemoveBinds();

    //create statement object
    m_stmt = mysql_stmt_init((MYSQL*)m_pMySQLConn);
    if (!m_stmt)
    {
        /*SYSTEM_LOG*/printf("SQL: mysql_stmt_init() failed \n");
        return false;
    }

    //prepare statement
    if (mysql_stmt_prepare((MYSQL_STMT*)m_stmt, m_szFmt.c_str(), m_szFmt.length()))
    {
        std::stringstream ss;
        ss<< "SQL: mysql_stmt_prepare() failed for '" << m_szFmt.c_str() << "'";
        std::cout << ss.str() << std::endl;//SYSTEM_LOG(ss.str());
        ss.str("");
        ss<< "SQL ERROR: " << mysql_stmt_error((MYSQL_STMT*)m_stmt);
        std::cout << ss.str() << std::endl;//SYSTEM_LOG(ss.str());
        
        return false;
    }

    /* Get the parameter count from the statement */
    m_nParams = mysql_stmt_param_count((MYSQL_STMT*)m_stmt);

    /* Fetch result set meta information */
    m_pResultMetadata = mysql_stmt_result_metadata((MYSQL_STMT*)m_stmt);
    //if we do not have result metadata
    if (!m_pResultMetadata && strnicmp(m_szFmt.c_str(), "select", 6) == 0)
    {
        
       
        
        std::stringstream ss;
        ss<< "SQL: no meta information for '" << m_szFmt.c_str() << "'";
        std::cout << ss.str() << std::endl;//SYSTEM_LOG(ss.str());
        ss.str("");
        ss<< "SQL ERROR: " << mysql_stmt_error((MYSQL_STMT*)m_stmt);
        std::cout << ss.str() <<std::endl;//YSTEM_LOG(ss.str());
        return false;
    }

    //bind input buffers
    if(m_nParams)
    {
        m_pInputArgs = new MYSQL_BIND[m_nParams];
        memset(m_pInputArgs, 0, sizeof(MYSQL_BIND) * m_nParams);
    }

    //check if we have a statement which returns result sets
    if((MYSQL_RES*)m_pResultMetadata)
    {
        //our statement is query
        m_bIsQuery = true;
        /* Get total columns in the query */
        m_nColumns = mysql_num_fields((MYSQL_RES*)m_pResultMetadata);

        //bind output buffers
    }

    m_bPrepared = true;
    return true;
}

void MySqlPreparedStatement::bind( const SqlStmtParameters& holder )
{
    if(!isPrepared())
    {
        assert(false);
        return;
    }

    //finalize adding params
    if(!m_pInputArgs)
        return;

    //verify if we bound all needed input parameters
    if(m_nParams != holder.boundParams())
    {
        assert(false);
        return;
    }

    int nIndex = 0;
    SqlStmtParameters::ParameterContainer const& _args = holder.params();

    SqlStmtParameters::ParameterContainer::const_iterator iter_last = _args.end();
    for (SqlStmtParameters::ParameterContainer::const_iterator iter = _args.begin(); iter != iter_last; ++iter)
    {
        //bind parameter
        addParam(nIndex++, (*iter));
    }

    //bind input arguments
    if(mysql_stmt_bind_param((MYSQL_STMT*)m_stmt, (MYSQL_BIND*)m_pInputArgs))
    {
        std::stringstream ss;
        /*SYSTEM_LOG*/printf("SQL ERROR: mysql_stmt_bind_param() failed\n");
        ss << "SQL ERROR: " << mysql_stmt_error((MYSQL_STMT*)m_stmt)<<std::endl;
        std::cout << ss.str() << std::endl;//SYSTEM_LOG(ss.str());
    }
}

void MySqlPreparedStatement::addParam( int nIndex, const SqlStmtFieldData& data )
{
    assert(m_pInputArgs);
    assert((unsigned int)nIndex < m_nParams);
	MYSQL_BIND* tmp =(MYSQL_BIND*) m_pInputArgs;

    MYSQL_BIND& pData = tmp[nIndex];

    bool bUnsigned = 0;
    MysqlTypes dataType = ToMySQLType(data, bUnsigned);

    //setup MYSQL_BIND structure
    pData.buffer_type =(enum_field_types) dataType;
    pData.is_unsigned = bUnsigned;
    pData.buffer = data.buff();
    pData.length = 0;
    pData.buffer_length = data.type() == FIELD_STRING ? data.size() : 0;
}

void MySqlPreparedStatement::RemoveBinds()
{
    if(!(MYSQL_STMT*)m_stmt)
        return;

    delete [] (MYSQL_BIND*)m_pInputArgs;
    delete [] m_pResult;

    mysql_free_result((MYSQL_RES*)m_pResultMetadata);
    mysql_stmt_close((MYSQL_STMT*)m_stmt);

    m_stmt = NULL;
    m_pResultMetadata = NULL;
    m_pResult = NULL;
    m_pInputArgs = NULL;

    m_bPrepared = false;
}

bool MySqlPreparedStatement::execute()
{
    if(!isPrepared())
        return false;

    if(mysql_stmt_execute((MYSQL_STMT*)m_stmt))
    {
        
        std::stringstream ss;
        ss<< "SQL: cannot execute '" << m_szFmt.c_str() << "'";
        std::cout << ss.str()<<std::endl;//SYSTEM_LOG(ss.str());
        ss.str("");
        ss<< "SQL ERROR: " << mysql_stmt_error((MYSQL_STMT*)m_stmt);
        std::cout << ss.str() << std::endl;//SYSTEM_LOG(ss.str());
        return false;
    }

    return true;
}

MysqlTypes MySqlPreparedStatement::ToMySQLType( const SqlStmtFieldData &data, bool &bUnsigned )
{
    bUnsigned = 0;
    enum_field_types dataType = MYSQL_TYPE_NULL;

    switch (data.type())
    {
    case FIELD_NONE:    dataType = MYSQL_TYPE_NULL;                     break;
    // MySQL does not support MYSQL_TYPE_BIT as input type
    case FIELD_BOOL:    //dataType = MYSQL_TYPE_BIT;      bUnsigned = 1;  break;
    case FIELD_UI8:     dataType = MYSQL_TYPE_TINY;     bUnsigned = 1;  break;
    case FIELD_I8:      dataType = MYSQL_TYPE_TINY;                     break;
    case FIELD_I16:     dataType = MYSQL_TYPE_SHORT;                    break;
    case FIELD_UI16:    dataType = MYSQL_TYPE_SHORT;    bUnsigned = 1;  break;
    case FIELD_I32:     dataType = MYSQL_TYPE_LONG;                     break;
    case FIELD_UI32:    dataType = MYSQL_TYPE_LONG;     bUnsigned = 1;  break;
    case FIELD_I64:     dataType = MYSQL_TYPE_LONGLONG;                 break;
    case FIELD_UI64:    dataType = MYSQL_TYPE_LONGLONG; bUnsigned = 1;  break;
    case FIELD_FLOAT:   dataType = MYSQL_TYPE_FLOAT;                    break;
    case FIELD_DOUBLE:  dataType = MYSQL_TYPE_DOUBLE;                   break;
    case FIELD_STRING:  dataType = MYSQL_TYPE_STRING;                   break;
    }

    return (MysqlTypes)dataType;
}
