#include "Database/GDBCSqlDelayThread.h"
#include "Database/GDBCSqlOperations.h"

#include "Database/GDBCField.h"
#include "Database/GDBCQueryResult.h"
#include "Database/GDBCSqlOperations.h"

SqlDelayThread::SqlDelayThread(Database* db, SqlConnection* conn) : m_dbEngine(db),
	m_dbConnection(conn), m_running(true)
{
}

SqlDelayThread::~SqlDelayThread()
{
    //process all requests which might have been queued while thread was stopping
    ProcessRequests();
}

void SqlDelayThread::run()
{
    #ifndef DO_POSTGRESQL
   // mysql_thread_init();
    #endif

    const GDBC::uint32 loopSleepms = 10;

    const GDBC::uint32 pingEveryLoop = m_dbEngine->GetPingIntervall() / loopSleepms;

    GDBC::uint32 loopCounter = 0;
    while (m_running)
    {
        // if the running state gets turned off while sleeping
        // empty the queue before exiting
        ACE_Based::Thread::Sleep(loopSleepms);

        ProcessRequests();

        if((loopCounter++) >= pingEveryLoop)
        {
            loopCounter = 0;
            m_dbEngine->Ping();
        }
    }

    #ifndef DO_POSTGRESQL
    // mysql_thread_end();
    #endif
}

void SqlDelayThread::Stop()
{
    m_running = false;
}

void SqlDelayThread::ProcessRequests()
{
    SqlOperation* s = NULL;
    while (m_sqlQueue.next(s))
    {
        s->Execute(m_dbConnection);
        delete s;
    }
}
