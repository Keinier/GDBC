#ifndef __GDBC_GDBCSQLOPERATIONS_H__
#define __GDBC_GDBCSQLOPERATIONS_H__

#include "GDBCPrerequisities.h"
#include "Database/GDBCDatabase.h"
#include "GDBCLockedQueue.h"
#include "GDBCCallback.h"

#include "ace/Thread_Mutex.h"

/// ---- BASE ---

class Database;
class SqlConnection;
class SqlDelayThread;
class SqlStmtParameters;

class SqlOperation
{
    public:
        virtual void OnRemove() { delete this; }
        virtual bool Execute(SqlConnection *conn) = 0;
        virtual ~SqlOperation() {}
};

/// ---- ASYNC STATEMENTS / TRANSACTIONS ----

class SqlPlainRequest : public SqlOperation
{
    private:
        const char *m_sql;
    public:
        SqlPlainRequest(const char *sql) : m_sql(gdbc_strdup(sql)){}
        ~SqlPlainRequest() { char* tofree = const_cast<char*>(m_sql); delete [] tofree; }
        bool Execute(SqlConnection *conn);
};

class SqlTransaction : public SqlOperation
{
    private:
        std::vector<SqlOperation * > m_queue;

    public:
        SqlTransaction() {}
        ~SqlTransaction();

        void DelayExecute(SqlOperation * sql)   {   m_queue.push_back(sql); }

        bool Execute(SqlConnection *conn);
};

class SqlPreparedRequest : public SqlOperation
{
    public:
        SqlPreparedRequest(int nIndex, SqlStmtParameters * arg);
        ~SqlPreparedRequest();

        bool Execute(SqlConnection *conn);

    private:
        const int m_nIndex;
        SqlStmtParameters * m_param;
};

/// ---- ASYNC QUERIES ----

class SqlQuery;                                             /// contains a single async query
class QueryResult;                                          /// the result of one
class SqlResultQueue;                                       /// queue for thread sync
class SqlQueryHolder;                                       /// groups several async quries
class SqlQueryHolderEx;                                     /// points to a holder, added to the delay thread

class SqlResultQueue : public ACE_Based::LockedQueue<GDBC::IQueryCallback* , ACE_Thread_Mutex>
{
    public:
        SqlResultQueue() {}
        void Update();
};

class SqlQuery : public SqlOperation
{
    private:
        const char *m_sql;
        GDBC::IQueryCallback * m_callback;
        SqlResultQueue * m_queue;
    public:
        SqlQuery(const char *sql, GDBC::IQueryCallback * callback, SqlResultQueue * queue)
            : m_sql(gdbc_strdup(sql)), m_callback(callback), m_queue(queue) {}
        ~SqlQuery() { char* tofree = const_cast<char*>(m_sql); delete [] tofree; }
        bool Execute(SqlConnection *conn);
};

class SqlQueryHolder
{
    friend class SqlQueryHolderEx;
    private:
        typedef std::pair<const char*, QueryResult*> SqlResultPair;
        std::vector<SqlResultPair> m_queries;
    public:
        SqlQueryHolder() {}
        ~SqlQueryHolder();
        bool SetQuery(size_t index, const char *sql);
        bool SetPQuery(size_t index, const char *format, ...);
        void SetSize(size_t size);
        QueryResult* GetResult(size_t index);
        void SetResult(size_t index, QueryResult *result);
        bool Execute(GDBC::IQueryCallback * callback, SqlDelayThread *thread, SqlResultQueue *queue);
};

class SqlQueryHolderEx : public SqlOperation
{
    private:
        SqlQueryHolder * m_holder;
        GDBC::IQueryCallback * m_callback;
        SqlResultQueue * m_queue;
    public:
        SqlQueryHolderEx(SqlQueryHolder *holder, GDBC::IQueryCallback * callback, SqlResultQueue * queue)
            : m_holder(holder), m_callback(callback), m_queue(queue) {}
        bool Execute(SqlConnection *conn);
};

#endif // __GDBC_GDBCSQLOPERATIONS_H__
