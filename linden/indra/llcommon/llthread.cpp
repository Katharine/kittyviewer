/** 
 * @file llthread.cpp
 *
 * $LicenseInfo:firstyear=2004&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "linden_common.h"
#include "llapr.h"

#include "apr_portable.h"

#include "llthread.h"

#include "lltimer.h"

#if LL_LINUX || LL_SOLARIS
#include <sched.h>
#endif

//----------------------------------------------------------------------------
// Usage:
// void run_func(LLThread* thread)
// {
// }
// LLThread* thread = new LLThread();
// thread->run(run_func);
// ...
// thread->setQuitting();
// while(!timeout)
// {
//   if (thread->isStopped())
//   {
//     delete thread;
//     break;
//   }
// }
// 
//----------------------------------------------------------------------------

//
// Handed to the APR thread creation function
//
void *APR_THREAD_FUNC LLThread::staticRun(apr_thread_t *apr_threadp, void *datap)
{
	LLThread *threadp = (LLThread *)datap;

	// Set thread state to running
	threadp->mStatus = RUNNING;

	// Run the user supplied function
	threadp->run();

	llinfos << "LLThread::staticRun() Exiting: " << threadp->mName << llendl;
	
	// We're done with the run function, this thread is done executing now.
	threadp->mStatus = STOPPED;

	return NULL;
}


LLThread::LLThread(const std::string& name, apr_pool_t *poolp) :
	mPaused(FALSE),
	mName(name),
	mAPRThreadp(NULL),
	mStatus(STOPPED)
{
	// Thread creation probably CAN be paranoid about APR being initialized, if necessary
	if (poolp)
	{
		mIsLocalPool = FALSE;
		mAPRPoolp = poolp;
	}
	else
	{
		mIsLocalPool = TRUE;
		apr_pool_create(&mAPRPoolp, NULL); // Create a subpool for this thread
	}
	mRunCondition = new LLCondition(mAPRPoolp);

	mLocalAPRFilePoolp = NULL ;
}


LLThread::~LLThread()
{
	shutdown();

	if(mLocalAPRFilePoolp)
	{
		delete mLocalAPRFilePoolp ;
		mLocalAPRFilePoolp = NULL ;
	}
}

void LLThread::shutdown()
{
	// Warning!  If you somehow call the thread destructor from itself,
	// the thread will die in an unclean fashion!
	if (mAPRThreadp)
	{
		if (!isStopped())
		{
			// The thread isn't already stopped
			// First, set the flag that indicates that we're ready to die
			setQuitting();

			llinfos << "LLThread::~LLThread() Killing thread " << mName << " Status: " << mStatus << llendl;
			// Now wait a bit for the thread to exit
			// It's unclear whether I should even bother doing this - this destructor
			// should netver get called unless we're already stopped, really...
			S32 counter = 0;
			const S32 MAX_WAIT = 600;
			while (counter < MAX_WAIT)
			{
				if (isStopped())
				{
					break;
				}
				// Sleep for a tenth of a second
				ms_sleep(100);
				yield();
				counter++;
			}
		}

		if (!isStopped())
		{
			// This thread just wouldn't stop, even though we gave it time
			llwarns << "LLThread::~LLThread() exiting thread before clean exit!" << llendl;
			return;
		}
		mAPRThreadp = NULL;
	}

	delete mRunCondition;
	
	if (mIsLocalPool)
	{
		apr_pool_destroy(mAPRPoolp);
	}
}


void LLThread::start()
{
	apr_thread_create(&mAPRThreadp, NULL, staticRun, (void *)this, mAPRPoolp);	

	// We won't bother joining
	apr_thread_detach(mAPRThreadp);
}

//============================================================================
// Called from MAIN THREAD.

// Request that the thread pause/resume.
// The thread will pause when (and if) it calls checkPause()
void LLThread::pause()
{
	if (!mPaused)
	{
		// this will cause the thread to stop execution as soon as checkPause() is called
		mPaused = 1;		// Does not need to be atomic since this is only set/unset from the main thread
	}	
}

void LLThread::unpause()
{
	if (mPaused)
	{
		mPaused = 0;
	}

	wake(); // wake up the thread if necessary
}

// virtual predicate function -- returns true if the thread should wake up, false if it should sleep.
bool LLThread::runCondition(void)
{
	// by default, always run.  Handling of pause/unpause is done regardless of this function's result.
	return true;
}

//============================================================================
// Called from run() (CHILD THREAD).
// Stop thread execution if requested until unpaused.
void LLThread::checkPause()
{
	mRunCondition->lock();

	// This is in a while loop because the pthread API allows for spurious wakeups.
	while(shouldSleep())
	{
		mRunCondition->wait(); // unlocks mRunCondition
		// mRunCondition is locked when the thread wakes up
	}
	
 	mRunCondition->unlock();
}

//============================================================================

void LLThread::setQuitting()
{
	mRunCondition->lock();
	if (mStatus == RUNNING)
	{
		mStatus = QUITTING;
	}
	mRunCondition->unlock();
	wake();
}

// static
U32 LLThread::currentID()
{
	return (U32)apr_os_thread_current();
}

// static
void LLThread::yield()
{
#if LL_LINUX || LL_SOLARIS
	sched_yield(); // annoyingly, apr_thread_yield  is a noop on linux...
#else
	apr_thread_yield();
#endif
}

void LLThread::wake()
{
	mRunCondition->lock();
	if(!shouldSleep())
	{
		mRunCondition->signal();
	}
	mRunCondition->unlock();
}

void LLThread::wakeLocked()
{
	if(!shouldSleep())
	{
		mRunCondition->signal();
	}
}

//============================================================================

LLMutex::LLMutex(apr_pool_t *poolp) :
	mAPRMutexp(NULL)
{
	//if (poolp)
	//{
	//	mIsLocalPool = FALSE;
	//	mAPRPoolp = poolp;
	//}
	//else
	{
		mIsLocalPool = TRUE;
		apr_pool_create(&mAPRPoolp, NULL); // Create a subpool for this thread
	}
	apr_thread_mutex_create(&mAPRMutexp, APR_THREAD_MUTEX_UNNESTED, mAPRPoolp);
}


LLMutex::~LLMutex()
{
#if MUTEX_DEBUG
	llassert_always(!isLocked()); // better not be locked!
#endif
	apr_thread_mutex_destroy(mAPRMutexp);
	mAPRMutexp = NULL;
	if (mIsLocalPool)
	{
		apr_pool_destroy(mAPRPoolp);
	}
}


void LLMutex::lock()
{
	apr_thread_mutex_lock(mAPRMutexp);
#if MUTEX_DEBUG
	// Have to have the lock before we can access the debug info
	U32 id = LLThread::currentID();
	if (mIsLocked[id] != FALSE)
		llerrs << "Already locked in Thread: " << id << llendl;
	mIsLocked[id] = TRUE;
#endif
}

void LLMutex::unlock()
{
#if MUTEX_DEBUG
	// Access the debug info while we have the lock
	U32 id = LLThread::currentID();
	if (mIsLocked[id] != TRUE)
		llerrs << "Not locked in Thread: " << id << llendl;	
	mIsLocked[id] = FALSE;
#endif
	apr_thread_mutex_unlock(mAPRMutexp);
}

bool LLMutex::isLocked()
{
	apr_status_t status = apr_thread_mutex_trylock(mAPRMutexp);
	if (APR_STATUS_IS_EBUSY(status))
	{
		return true;
	}
	else
	{
		apr_thread_mutex_unlock(mAPRMutexp);
		return false;
	}
}

//============================================================================

LLCondition::LLCondition(apr_pool_t *poolp) :
	LLMutex(poolp)
{
	// base class (LLMutex) has already ensured that mAPRPoolp is set up.

	apr_thread_cond_create(&mAPRCondp, mAPRPoolp);
}


LLCondition::~LLCondition()
{
	apr_thread_cond_destroy(mAPRCondp);
	mAPRCondp = NULL;
}


void LLCondition::wait()
{
	apr_thread_cond_wait(mAPRCondp, mAPRMutexp);
}

void LLCondition::signal()
{
	apr_thread_cond_signal(mAPRCondp);
}

void LLCondition::broadcast()
{
	apr_thread_cond_broadcast(mAPRCondp);
}

//============================================================================

//----------------------------------------------------------------------------

//static
LLMutex* LLThreadSafeRefCount::sMutex = 0;

//static
void LLThreadSafeRefCount::initThreadSafeRefCount()
{
	if (!sMutex)
	{
		sMutex = new LLMutex(0);
	}
}

//static
void LLThreadSafeRefCount::cleanupThreadSafeRefCount()
{
	delete sMutex;
	sMutex = NULL;
}
	

//----------------------------------------------------------------------------

LLThreadSafeRefCount::LLThreadSafeRefCount() :
	mRef(0)
{
}

LLThreadSafeRefCount::~LLThreadSafeRefCount()
{ 
	if (mRef != 0)
	{
		llerrs << "deleting non-zero reference" << llendl;
	}
}

//============================================================================

LLResponder::~LLResponder()
{
}

//============================================================================
