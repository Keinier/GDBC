/******************************************************************************
*                                                                             *
*       File Name: GDBCThreading.h                                            *
*       Description: Define a Runnable's Structure.                           *
*                                                                             *
*       Author: Keinier Caboverde Ramírez. <keinier@gmail.com>                *
*       Date: 03/05/2013 08:51 a.m.                                           *
*       File's Version: 1.3.1                                                 *
*                                                                             *
******************************************************************************/
#ifndef __GDBC_GDBCTHREADING_H__
#define __GDBC_GDBCTHREADING_H__
#include "GDBCPrerequisities.h"
#include <ace/Thread.h>
#include <ace/TSS_T.h>
#include "ace/Atomic_Op.h"
#include <assert.h>

namespace ACE_Based
{
    class Runnable
    {
        public:
            virtual ~Runnable() {}
            virtual void run() = 0;

            void incReference() { ++m_refs; }
            void decReference()
            {
                if(!--m_refs)
                    delete this;
            }
        private:
            ACE_Atomic_Op<ACE_Thread_Mutex, long> m_refs;
    };

    enum Priority
    {
        Idle,
        Lowest,
        Low,
        Normal,
        High,
        Highest,
        Realtime,
    };

#define MAXPRIORITYNUM (Realtime + 1)

    class ThreadPriority
    {
        public:
            ThreadPriority();
            int getPriority(Priority p) const;

        private:
            int m_priority[MAXPRIORITYNUM];
    };

    class Thread
    {
        public:
            Thread();
            explicit Thread(Runnable* instance);
            ~Thread();

            bool start();
            bool wait();
            void destroy();

            void suspend();
            void resume();

            void setPriority(Priority type);

            static void Sleep(unsigned long msecs);
            static ACE_thread_t currentId();
            static ACE_hthread_t currentHandle();
            static Thread * current();

        private:
            Thread(const Thread&);
            Thread& operator=(const Thread&);

            static ACE_THR_FUNC_RETURN ThreadTask(void * param);

            ACE_thread_t m_iThreadId;
            ACE_hthread_t m_hThreadHandle;
            Runnable * m_task;

            typedef ACE_TSS<Thread> ThreadStorage;
            //global object - container for Thread class representation of every thread
            static ThreadStorage m_ThreadStorage;
            //use this object to determine current OS thread priority values mapped to enum Priority{}
            static ThreadPriority m_TpEnum;
    };
}


namespace GDBC
{
    template<typename MUTEX>
	class _GDBCAPIExport GeneralLock
    {
        public:

            GeneralLock(MUTEX &m)
                : i_mutex(m)
            {
                i_mutex.acquire();
            }

            ~GeneralLock()
            {
                i_mutex.release();
            }

        private:

            GeneralLock(const GeneralLock &);
            GeneralLock& operator=(const GeneralLock &);
            MUTEX &i_mutex;
    };

    template<class T>
    class _GDBCAPIExport SingleThreaded
    {
        public:

            struct Lock                                     // empty object
            {
                Lock()
                {
                }
                Lock(const T&)
                {
                }

                Lock(const SingleThreaded<T>&)              // for single threaded we ignore this
                {
                }
            };
    };

    template<class T, class MUTEX>
    class _GDBCAPIExport ObjectLevelLockable
    {
        public:

            ObjectLevelLockable()
                : i_mtx()
            {
            }

            friend class Lock;

            class Lock
            {
                public:

                    Lock(ObjectLevelLockable<T, MUTEX> &host)
                        : i_lock(host.i_mtx)
                    {
                    }

                private:

                    GeneralLock<MUTEX> i_lock;
            };

        private:

            // prevent the compiler creating a copy construct
            ObjectLevelLockable(const ObjectLevelLockable<T, MUTEX>&);
            ObjectLevelLockable<T, MUTEX>& operator=(const ObjectLevelLockable<T, MUTEX>&);

            MUTEX i_mtx;
    };

    template<class T, class MUTEX>
    class _GDBCAPIExport ClassLevelLockable
    {
        public:

            ClassLevelLockable()
            {
            }

            friend class Lock;

            class Lock
            {
                public:

                    Lock(const T& /*host*/)
                    {
                        ClassLevelLockable<T, MUTEX>::si_mtx.acquire();
                    }

                    Lock(const ClassLevelLockable<T, MUTEX> &)
                    {
                        ClassLevelLockable<T, MUTEX>::si_mtx.acquire();
                    }

                    Lock()
                    {
                        ClassLevelLockable<T, MUTEX>::si_mtx.acquire();
                    }

                    ~Lock()
                    {
                        ClassLevelLockable<T, MUTEX>::si_mtx.release();
                    }
            };

        private:

            static MUTEX si_mtx;
    };

}

template<class T, class MUTEX> MUTEX GDBC::ClassLevelLockable<T, MUTEX>::si_mtx;

#define INSTANTIATE_CLASS_MUTEX(CTYPE, MUTEX) \
    template class _GDBCAPIExport GDBC::ClassLevelLockable<CTYPE, MUTEX>

#endif // __GDBC_GDBCTHREADING_H__