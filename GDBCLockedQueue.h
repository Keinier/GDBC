#ifndef __GDBC_GDBCLOCKEDQUEUE_H__
#define __GDBC_GDBCLOCKEDQUEUE_H__
#include "GDBCPrerequisities.h"

#include <ace/Guard_T.h>
#include <ace/Thread_Mutex.h>

namespace ACE_Based
{
    template <class T, class LockType, typename StorageType=std::deque<T> >
        class LockedQueue
    {
        //! Lock access to the queue.
        LockType _lock;

        //! Storage backing the queue.
        StorageType _queue;

        //! Cancellation flag.
        /*volatile*/ bool _canceled;

        public:

            //! Create a LockedQueue.
            LockedQueue()
                : _canceled(false)
            {
            }

            //! Destroy a LockedQueue.
            virtual ~LockedQueue()
            {
            }

            //! Adds an item to the queue.
            void add(const T& item)
            {
                ACE_Guard<LockType> g(this->_lock);
                _queue.push_back(item);
            }

            //! Gets the next result in the queue, if any.
            bool next(T& result)
            {
                ACE_GUARD_RETURN (LockType, g, this->_lock, false);

                if (_queue.empty())
                    return false;

                result = _queue.front();
                _queue.pop_front();

                return true;
            }

            template<class Checker>
            bool next(T& result, Checker& check)
            {
                ACE_GUARD_RETURN (LockType, g, this->_lock, false);

                if (_queue.empty())
                    return false;

                result = _queue.front();
                if(!check.Process(result))
                    return false;

                _queue.pop_front();
                return true;
            }

            //! Peeks at the top of the queue. Remember to unlock after use.
            T& peek()
            {
                lock();

                T& result = _queue.front();

                return result;
            }

            //! Cancels the queue.
            void cancel()
            {
                ACE_Guard<LockType> g(this->_lock);
                _canceled = true;
            }

            //! Checks if the queue is cancelled.
            bool cancelled()
            {
                ACE_Guard<LockType> g(this->_lock);
                return _canceled;
            }

            //! Locks the queue for access.
            void lock()
            {
                this->_lock.acquire();
            }

            //! Unlocks the queue.
            void unlock()
            {
                this->_lock.release();
            }

            ///! Checks if we're empty or not with locks held
            bool empty()
            {
                ACE_Guard<LockType> g(this->_lock);
                return _queue.empty();
            }
    };
}
#endif