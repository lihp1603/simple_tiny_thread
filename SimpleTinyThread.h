/************************************************************************/
/*funtion: 实现简单的跨平台多线程(window,linux)
*author:lhp
*email:lihaiping1603@aliyun.com
*date:20160706
*Version:V1.0.0
*NOTE:simple tiny thread的代码实现主要是从tiny thread中裁剪和修改得到
	  tiny thread:http://tinythreadpp.bitsnbites.eu/
*/
/************************************************************************/

#ifndef __SIMPLE_TINY_THREAD_H
#define __SIMPLE_TINY_THREAD_H

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define _TTHREAD_WIN32_
#else
#define _TTHREAD_POSIX_
#endif


// Platform specific includes
#if defined(_TTHREAD_WIN32_)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define __UNDEF_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef __UNDEF_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#undef __UNDEF_LEAN_AND_MEAN
#endif
#else//posix
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>
#endif

#include "CCriticalLock.h"

namespace simple_tiny_thread{
	/// Thread class.
	class stthread {
	public:
#if defined(_TTHREAD_WIN32_)
		typedef HANDLE native_handle_type;
#else
		typedef pthread_t native_handle_type;
#endif
		//////////////////////////////////////////////////////////////////////////
		stthread(void (*aFunction)(void *), void * aArg);
		~stthread();

		void join();

		bool joinable() const;

		void detach();

		unsigned long int get_id() const;

		inline native_handle_type native_handle()
		{
			return mHandle;
		}
	private:
		native_handle_type mHandle;   ///< Thread handle.
		CCriticalSection mDataMutex;  ///< Serializer for access to the thread private data.
		bool mNotAThread;             ///< True if this object is not a thread of execution.
#if defined(_TTHREAD_WIN32_)
		unsigned int mWin32ThreadID;  ///< Unique thread ID (filled out by _beginthreadex).
#endif

		// This is the internal thread wrapper function.
#if defined(_TTHREAD_WIN32_)
		static unsigned WINAPI wrapper_function(void * aArg);
#else
		static void * wrapper_function(void * aArg);
#endif
	};
}


#endif

