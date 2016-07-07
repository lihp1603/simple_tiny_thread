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

#include "SimpleTinyThread.h"
#include "CCriticalLock.h"
#include <iostream> 

#if defined(_TTHREAD_POSIX_)
#include <unistd.h>
#include <map>
#elif defined(_TTHREAD_WIN32_)
#include <process.h>
#endif


namespace simple_tiny_thread{
	//------------------------------------------------------------------------------
	// POSIX pthread_t to unique thread::id mapping logic.
	// Note: Here we use a global thread safe std::map to convert instances of
	// pthread_t to small thread identifier numbers (unique within one process).
	// This method should be portable across different POSIX implementations.
	//------------------------------------------------------------------------------
#if defined(_TTHREAD_POSIX_)
	static unsigned long int _pthread_t_to_ID(const pthread_t &aHandle)
	{
		static CCriticalSection idMapLock;
		static std::map<pthread_t, unsigned long int> idMap;
		static unsigned long int idCount(1);

		CScopeCS scope(idMapLock);
		if(idMap.find(aHandle) == idMap.end())
			idMap[aHandle] = idCount ++;
		return idMap[aHandle];
	}
#endif // _TTHREAD_POSIX_

	/// Information to pass to the new stthread (what to run).
	typedef struct _tagThreadStartInfo_S {
		void (*mFunction)(void *); ///< Pointer to the function to be executed.
		void * mArg;               ///< Function argument for the stthread function.
		stthread * mThread;          ///< Pointer to the stthread object.
	}ThreadStartInfo_S,*lpThreadStartInfo_S;


	// Thread wrapper function.
	#if defined(_TTHREAD_WIN32_)
	unsigned WINAPI stthread::wrapper_function(void * aArg)
	#elif defined(_TTHREAD_POSIX_)
	void * stthread::wrapper_function(void * aArg)
	#endif
	{
		// Get stthread startup information
		ThreadStartInfo_S * pthreadi = (ThreadStartInfo_S *) aArg;

		try
		{
			// Call the actual client stthread function
			pthreadi->mFunction(pthreadi->mArg);
		}
		catch(...)
		{
			// Uncaught exceptions will terminate the application (default behavior
			// according to C++11)
			std::terminate();
		}

		// The stthread is no longer executing
		CScopeCS scope(pthreadi->mThread->mDataMutex);
		pthreadi->mThread->mNotAThread = true;

		// The stthread is responsible for freeing the startup information
		delete pthreadi;
		pthreadi=NULL;

		return 0;
	}

	stthread::stthread(void (*aFunction)(void *), void * aArg)
	{
		// Fill out the stthread startup information (passed to the stthread wrapper,
		// which will eventually free it)
		ThreadStartInfo_S * pthreadi = new ThreadStartInfo_S;
		if (pthreadi==NULL)
		{
			return;
		}
	
		pthreadi->mFunction = aFunction;
		pthreadi->mArg = aArg;
		pthreadi->mThread = this;

		// The stthread is now alive
		mNotAThread = false;

		// Create the stthread
	#if defined(_TTHREAD_WIN32_)
		mHandle = (HANDLE) _beginthreadex(0, 0, wrapper_function, (void *) pthreadi, 0, &mWin32ThreadID);
	#elif defined(_TTHREAD_POSIX_)
		if(pthread_create(&mHandle, NULL, wrapper_function, (void *) pthreadi) != 0)
			mHandle = 0;
	#endif

		// Did we fail to create the stthread?
		if(!mHandle)
		{
			mNotAThread = true;
			delete pthreadi;
			pthreadi=NULL;
		}
	}

	stthread::~stthread()
	{
		if(joinable())
			std::terminate();
	}

	void stthread::join()
	{
		if(joinable())
		{
	#if defined(_TTHREAD_WIN32_)
			WaitForSingleObject(mHandle, INFINITE);
			CloseHandle(mHandle);
	#elif defined(_TTHREAD_POSIX_)
			pthread_join(mHandle, NULL);
	#endif
		}
	}

	bool stthread::joinable() const
	{
		//CScopeCS scope(mDataMutex);
		//bool result = !mNotAThread;
		return !mNotAThread;
	}

	void stthread::detach()
	{
		CScopeCS scope(mDataMutex);
		if(!mNotAThread)
		{
	#if defined(_TTHREAD_WIN32_)
			CloseHandle(mHandle);
	#elif defined(_TTHREAD_POSIX_)
			pthread_detach(mHandle);
	#endif
			mNotAThread = true;
		}
	}

	unsigned long int stthread::get_id() const
	{
		if(!joinable())
			return 0;
	#if defined(_TTHREAD_WIN32_)
		return ((unsigned long int) mWin32ThreadID);
	#elif defined(_TTHREAD_POSIX_)
		return _pthread_t_to_ID(mHandle);
	#endif
	}
}