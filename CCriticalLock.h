/************************************************************************/
/* 
funtion:���ڶ��ٽ������м���,��linux��win������ʹ��
author: -lhp
date:	-20151224
*/
/************************************************************************/
#ifndef  __CCRITICALLOCK_H_
#define  __CCRITICALLOCK_H_
//////////////////////////////////////////////////////////////////////////
// scope locker
template<typename LockerType>
class CScopeLocker
{
public:
	explicit CScopeLocker(LockerType &locker)
		: m_locker(locker)
	{
		m_locker.Lock();
	}
	~CScopeLocker()
	{
		m_locker.UnLock();
	}	
private:
	CScopeLocker(const CScopeLocker& rhs);
	CScopeLocker& operator=(const CScopeLocker& rhs);
private:	
	LockerType &m_locker;
};	

#ifdef __AVCOM_WIN32_CRITICAL_SECTION    //����win32�����µ��ٽ�
//���������ͷ�ļ�
#include <windows.h>
class CCriticalSection
{
public:
	CCriticalSection()
	{
		InitializeCriticalSection(&m_pCrit);
	};
	~CCriticalSection()
	{

		DeleteCriticalSection(&m_pCrit);
	};
	void Lock()
	{
		EnterCriticalSection(&m_pCrit);
	};

	void UnLock()
	{

		LeaveCriticalSection(&m_pCrit);
	};

	void Enter() {  Lock();	}
	void Leave() {  UnLock();}


private:
	CRITICAL_SECTION m_pCrit;
};
//CWinScopeCS
typedef CScopeLocker<CCriticalSection> CScopeCS;

//Ϊ�˼���֮ǰ�Ķ���

#else//linux������
#include <pthread.h>
class CCriticalSection
{
public:
	CCriticalSection()
	{
		pthread_mutex_init(&m_pMutex,NULL);
	}
	~CCriticalSection()
	{
		pthread_mutex_destroy(&m_pMutex);
	}

	void Lock()
	{
		pthread_mutex_lock(&m_pMutex);
	}
	void UnLock()
	{
		pthread_mutex_unlock(&m_pMutex);
	}
	void Enter() {  Lock();	}
	void Leave() {  UnLock();}
private:
	pthread_mutex_t m_pMutex;
};
//CLinuxScopeCS
typedef CScopeLocker<CCriticalSection> CScopeCS;

#endif

#endif