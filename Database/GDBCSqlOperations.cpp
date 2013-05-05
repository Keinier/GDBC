#include "Database/GDBCSqlOperations.h"
#include "Database/GDBCSqlDelayThread.h"

#include "Database/GDBCField.h"
#include "Database/GDBCQueryResult.h"
#include "Database/GDBCSqlOperations.h"

#include "Database/GDBCDatabaseImpl.h"

#define LOCK_GDBC(conn) SqlConnection::Lock guard(conn)

/// ---- ASYNC STATEMENTS / TRANSACTIONS ----

bool SqlPlainRequest::Execute(SqlConnection *conn)
{
    /// just do it
    LOCK_GDBC(conn);
    return conn->Execute(m_sql);
}

SqlTransaction::~SqlTransaction()
{
    while(!m_queue.empty())
    {
        delete m_queue.back();
        m_queue.pop_back();
    }
}

bool SqlTransaction::Execute(SqlConnection *conn)
{
    if(m_queue.empty())
        return true;

    LOCK_GDBC(conn);

    conn->BeginTransaction();

    const int nItems = m_queue.size();
    for (int i = 0; i < nItems; ++i)
    {
        SqlOperation * pStmt = m_queue[i];

        if(!pStmt->Execute(conn))
        {
            conn->RollbackTransaction();
            return false;
        }
    }

    return conn->CommitTransaction();
}

SqlPreparedRequest::SqlPreparedRequest(int nIndex, SqlStmtParameters * arg ) : m_nIndex(nIndex), m_param(arg)
{
}

SqlPreparedRequest::~SqlPreparedRequest()
{
    delete m_param;
}

bool SqlPreparedRequest::Execute( SqlConnection *conn )
{
    LOCK_GDBC(conn);
    return conn->ExecuteStmt(m_nIndex, *m_param);
}

/// ---- ASYNC QUERIES ----

bool SqlQuery::Execute(SqlConnection *conn)
{
    if(!m_callback || !m_queue)
        return false;

    LOCK_GDBC(conn);
    /// execute the query and store the result in the callback
    m_callback->SetResult(conn->Query(m_sql));
    /// add the callback to the sql result queue of the thread it originated from
    m_queue->add(m_callback);

    return true;
}

void SqlResultQueue::Update()
{
    /// execute the callbacks waiting in the synchronization queue
    GDBC::IQueryCallback* callback = NULL;
    while (next(callback))
    {
        callback->Execute();
        delete callback;
    }
}

bool SqlQueryHolder::Execute(GDBC::IQueryCallback * callback, SqlDelayThread *thread, SqlResultQueue *queue)
{
    if(!callback || !thread || !queue)
        return false;

    /// delay the execution of the queries, sync them with the delay thread
    /// which will in turn resync on execution (via the queue) and call back
    SqlQueryHolderEx *holderEx = new SqlQueryHolderEx(this, callback, queue);
    thread->Delay(holderEx);
    return true;
}

bool SqlQueryHolder::SetQuery(size_t index, const char *sql)
{
    if(m_queries.size() <= index)
    {
        std::stringstream ss;
        ss << "Query index (" << index << ") out of range (size: " << m_queries.size() << ") for query: " << sql;
        std::cout << ss.str(); //SYSTEM_LOG(ss.str());
        return false;
    }

    if(m_queries[index].first != NULL)
    {
        std::stringstream ss;
        ss << "Attempt assign query to holder index ("  << index << ") where other query stored (Old: [" << m_queries[index].first << "] New: [" << sql << "])";
        std::cout << ss.str();   //SYSTEM_LOG(ss.str());
        return false;
    }

    /// not executed yet, just stored (it's not called a holder for nothing)
    m_queries[index] = SqlResultPair(gdbc_strdup(sql), (QueryResult*)NULL);
    return true;
}

bool SqlQueryHolder::SetPQuery(size_t index, const char *format, ...)
{
    if(!format)
    {
        std::stringstream ss;
        ss << "Query (index: "  << index << ") is empty.";
        std::cout << ss.str(); // SYSTEM_LOG(ss.str());
        return false;
    }

    va_list ap;
    char szQuery [MAX_QUERY_LEN];
    va_start(ap, format);
    int res = vsnprintf( szQuery, MAX_QUERY_LEN, format, ap );
    va_end(ap);

    if(res==-1)
    {
        std::stringstream ss;
        ss << "SQL Query truncated (and not execute) for format: "  << format;
        std::cout << ss.str(); // SYSTEM_LOG(ss.str());
        return false;
    }

    return SetQuery(index,szQuery);
}

QueryResult* SqlQueryHolder::GetResult(size_t index)
{
    if(index < m_queries.size())
    {
        /// the query strings are freed on the first GetResult or in the destructor
        if(m_queries[index].first != NULL)
        {
            delete [] (const_cast<char*>(m_queries[index].first));
            m_queries[index].first = NULL;
        }
        /// when you get a result aways remember to delete it!
        return m_queries[index].second;
    }
    else
        return NULL;
}

void SqlQueryHolder::SetResult(size_t index, QueryResult *result)
{
    /// store the result in the holder
    if(index < m_queries.size())
        m_queries[index].second = result;
}

SqlQueryHolder::~SqlQueryHolder()
{
    for(size_t i = 0; i < m_queries.size(); i++)
    {
        /// if the result was never used, free the resources
        /// results used already (getresult called) are expected to be deleted
        if(m_queries[i].first != NULL)
        {
            delete [] (const_cast<char*>(m_queries[i].first));
            if(m_queries[i].second)
                delete m_queries[i].second;
        }
    }
}

void SqlQueryHolder::SetSize(size_t size)
{
    /// to optimize push_back, reserve the number of queries about to be executed
    m_queries.resize(size);
}

bool SqlQueryHolderEx::Execute(SqlConnection *conn)
{
    if(!m_holder || !m_callback || !m_queue)
        return false;

    LOCK_GDBC(conn);
    /// we can do this, we are friends
    std::vector<SqlQueryHolder::SqlResultPair> &queries = m_holder->m_queries;
    for(size_t i = 0; i < queries.size(); i++)
    {
        /// execute all queries in the holder and pass the results
        char const *sql = queries[i].first;
        if(sql) m_holder->SetResult(i, conn->Query(sql));
    }

    /// sync with the caller thread
    m_queue->add(m_callback);

    return true;
}
