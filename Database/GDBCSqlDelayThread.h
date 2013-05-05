#ifndef __GDBC_GDBCSQLDELAYTHREAD_H__
#define __GDBC_GDBCSQLDELAYTHREAD_H__

#include "GDBCPrerequisities.h"
#include "ace/Thread_Mutex.h"
#include "GDBCLockedQueue.h"
#include "GDBCThreading.h"


class Database;
class SqlOperation;
class SqlConnection;

class SqlDelayThread : public ACE_Based::Runnable
{
    typedef ACE_Based::LockedQueue<SqlOperation*, ACE_Thread_Mutex> SqlQueue;

    private:
        SqlQueue m_sqlQueue;                                ///< Queue of SQL statements
        Database* m_dbEngine;                               ///< Pointer to used Database engine
        SqlConnection * m_dbConnection;                     ///< Pointer to DB connection
        volatile bool m_running;

        //process all enqueued requests
        void ProcessRequests();

    public:
        SqlDelayThread(Database* db, SqlConnection* conn);
        ~SqlDelayThread();

        ///< Put sql statement to delay queue
        bool Delay(SqlOperation* sql) { m_sqlQueue.add(sql); return true; }

        virtual void Stop();                                ///< Stop event
        virtual void run();                                 ///< Main Thread loop
};
#endif //__GDBC_GDBCSQLDELAYTHREAD_H__
