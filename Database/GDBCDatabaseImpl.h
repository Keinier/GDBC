#include "Database/GDBCDatabase.h"
#include "Database/GDBCSqlOperations.h"
#include <string>
/// Function body definitions for the template function members of the Database class

#define ASYNC_QUERY_BODY(sql) if (!sql || !m_pResultQueue) return false;
#define ASYNC_DELAYHOLDER_BODY(holder) if (!holder || !m_pResultQueue) return false;

#define ASYNC_PQUERY_BODY(format, szQuery) \
    if(!format) return false; \
    \
    char szQuery [MAX_QUERY_LEN]; \
    \
    { \
        va_list ap; \
        \
        va_start(ap, format); \
        int res = vsnprintf( szQuery, MAX_QUERY_LEN, format, ap ); \
        va_end(ap); \
        \
        if(res==-1) \
        { \
          std::stringstream ss; \
          ss << "SQL Query truncated (and not execute) for format: "<< format;\
            SYSTEM_LOG(ss.str()); \
            return false; \
        } \
    }

// -- Query / member --

template<class Class>
bool
Database::AsyncQuery(Class *object, void (Class::*method)(QueryResult*), const char *sql)
{
    ASYNC_QUERY_BODY(sql)
    return m_threadBody->Delay(new SqlQuery(sql, new MaNGOS::QueryCallback<Class>(object, method), m_pResultQueue));
}

template<class Class, typename ParamType1>
bool
Database::AsyncQuery(Class *object, void (Class::*method)(QueryResult*, ParamType1), ParamType1 param1, const char *sql)
{
    ASYNC_QUERY_BODY(sql)
    return m_threadBody->Delay(new SqlQuery(sql, new MaNGOS::QueryCallback<Class, ParamType1>(object, method, (QueryResult*)NULL, param1), m_pResultQueue));
}

template<class Class, typename ParamType1, typename ParamType2>
bool
Database::AsyncQuery(Class *object, void (Class::*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char *sql)
{
    ASYNC_QUERY_BODY(sql)
    return m_threadBody->Delay(new SqlQuery(sql, new MaNGOS::QueryCallback<Class, ParamType1, ParamType2>(object, method, (QueryResult*)NULL, param1, param2), m_pResultQueue));
}

template<class Class, typename ParamType1, typename ParamType2, typename ParamType3>
bool
Database::AsyncQuery(Class *object, void (Class::*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char *sql)
{
    ASYNC_QUERY_BODY(sql)
    return m_threadBody->Delay(new SqlQuery(sql, new MaNGOS::QueryCallback<Class, ParamType1, ParamType2, ParamType3>(object, method, (QueryResult*)NULL, param1, param2, param3), m_pResultQueue));
}

// -- Query / static --

template<typename ParamType1>
bool
Database::AsyncQuery(void (*method)(QueryResult*, ParamType1), ParamType1 param1, const char *sql)
{
    ASYNC_QUERY_BODY(sql)
    return m_threadBody->Delay(new SqlQuery(sql, new MaNGOS::SQueryCallback<ParamType1>(method, (QueryResult*)NULL, param1), m_pResultQueue));
}

template<typename ParamType1, typename ParamType2>
bool
Database::AsyncQuery(void (*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char *sql)
{
    ASYNC_QUERY_BODY(sql)
    return m_threadBody->Delay(new SqlQuery(sql, new MaNGOS::SQueryCallback<ParamType1, ParamType2>(method, (QueryResult*)NULL, param1, param2), m_pResultQueue));
}

template<typename ParamType1, typename ParamType2, typename ParamType3>
bool
Database::AsyncQuery(void (*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char *sql)
{
    ASYNC_QUERY_BODY(sql)
    return m_threadBody->Delay(new SqlQuery(sql, new MaNGOS::SQueryCallback<ParamType1, ParamType2, ParamType3>(method, (QueryResult*)NULL, param1, param2, param3), m_pResultQueue));
}

// -- PQuery / member --

template<class Class>
bool
Database::AsyncPQuery(Class *object, void (Class::*method)(QueryResult*), const char *format,...)
{
    ASYNC_PQUERY_BODY(format, szQuery)
    return AsyncQuery(object, method, szQuery);
}

template<class Class, typename ParamType1>
bool
Database::AsyncPQuery(Class *object, void (Class::*method)(QueryResult*, ParamType1), ParamType1 param1, const char *format,...)
{
    ASYNC_PQUERY_BODY(format, szQuery)
    return AsyncQuery(object, method, param1, szQuery);
}

template<class Class, typename ParamType1, typename ParamType2>
bool
Database::AsyncPQuery(Class *object, void (Class::*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char *format,...)
{
    ASYNC_PQUERY_BODY(format, szQuery)
    return AsyncQuery(object, method, param1, param2, szQuery);
}

template<class Class, typename ParamType1, typename ParamType2, typename ParamType3>
bool
Database::AsyncPQuery(Class *object, void (Class::*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char *format,...)
{
    ASYNC_PQUERY_BODY(format, szQuery)
    return AsyncQuery(object, method, param1, param2, param3, szQuery);
}

// -- PQuery / static --

template<typename ParamType1>
bool
Database::AsyncPQuery(void (*method)(QueryResult*, ParamType1), ParamType1 param1, const char *format,...)
{
    ASYNC_PQUERY_BODY(format, szQuery)
    return AsyncQuery(method, param1, szQuery);
}

template<typename ParamType1, typename ParamType2>
bool
Database::AsyncPQuery(void (*method)(QueryResult*, ParamType1, ParamType2), ParamType1 param1, ParamType2 param2, const char *format,...)
{
    ASYNC_PQUERY_BODY(format, szQuery)
    return AsyncQuery(method, param1, param2, szQuery);
}

template<typename ParamType1, typename ParamType2, typename ParamType3>
bool
Database::AsyncPQuery(void (*method)(QueryResult*, ParamType1, ParamType2, ParamType3), ParamType1 param1, ParamType2 param2, ParamType3 param3, const char *format,...)
{
    ASYNC_PQUERY_BODY(format, szQuery)
    return AsyncQuery(method, param1, param2, param3, szQuery);
}

// -- QueryHolder --

template<class Class>
bool
Database::DelayQueryHolder(Class *object, void (Class::*method)(QueryResult*, SqlQueryHolder*), SqlQueryHolder *holder)
{
    ASYNC_DELAYHOLDER_BODY(holder)
    return holder->Execute(new MaNGOS::QueryCallback<Class, SqlQueryHolder*>(object, method, (QueryResult*)NULL, holder), m_threadBody, m_pResultQueue);
}

template<class Class, typename ParamType1>
bool
Database::DelayQueryHolder(Class *object, void (Class::*method)(QueryResult*, SqlQueryHolder*, ParamType1), SqlQueryHolder *holder, ParamType1 param1)
{
    ASYNC_DELAYHOLDER_BODY(holder)
    return holder->Execute(new MaNGOS::QueryCallback<Class, SqlQueryHolder*, ParamType1>(object, method, (QueryResult*)NULL, holder, param1), m_threadBody, m_pResultQueue);
}

#undef ASYNC_QUERY_BODY
#undef ASYNC_PQUERY_BODY
#undef ASYNC_DELAYHOLDER_BODY
