// Thread.h : wraps mutithreading components 

#ifndef _THREAD_H
#define _THREAD_H

#include <winbase.h>
#include <process.h>
#include <deque>
#include <algorithm>

namespace mtlib {

/////////////////////////////////////////////////////////////////////////////
class key_t
{
public:
	virtual bool enter() = 0;
	virtual bool leave() = 0;
};
		
class lock_t {
public:
	lock_t(key_t& key) : m_key(key) {	m_key.enter();	}
	~lock_t()						{	m_key.leave();	}
private:
	lock_t(const lock_t&);				// no impl
	lock_t& operator = (const lock_t&); // no impl
	key_t& m_key;
};

/////////////////////////////////////////////////////////////////////////////
class critsect_t : public CRITICAL_SECTION, public key_t {
public:
	critsect_t()			{ InitializeCriticalSection(this);	}
	~critsect_t()			{ DeleteCriticalSection(this);		}
	virtual bool enter()	{ EnterCriticalSection(this); return true; }
	virtual bool leave()	{ LeaveCriticalSection(this); return true; }
			bool tryenter()	{ return TryEnterCriticalSection(this) ? true : false; }
private:
	critsect_t(const critsect_t&);					// noimpl 
	critsect_t& operator = (const critsect_t&);		// noimpl 
};

typedef lock_t cslock_t; // for compatibility - used to be separate class

/////////////////////////////////////////////////////////////////////////////
class handle_t
{
public:
	handle_t() : m_handle(NULL) { }
	~handle_t() { close(); }
	operator HANDLE() const { return m_handle;		}
	void close() { if (m_handle) CloseHandle(m_handle); m_handle = NULL; }
protected:
	HANDLE	m_handle;
private:
	handle_t(const handle_t&);					// noimpl 
	handle_t& operator = (const handle_t&);		// noimpl 
};

/////////////////////////////////////////////////////////////////////////////
class mutex_t: public handle_t, public key_t
{
public:
	mutex_t(LPCTSTR lpName = NULL, BOOL bInitialOwner= FALSE,
			LPSECURITY_ATTRIBUTES lpMutexAttributes = NULL)
	{ 
		m_handle = CreateMutex(lpMutexAttributes, bInitialOwner, lpName);
	}
	virtual bool enter() { 
		if (m_handle)
			return WaitForSingleObject(m_handle,INFINITE) == WAIT_OBJECT_0; 
		return false;
	}
	virtual bool leave()	{ 
		if (m_handle)
			return ReleaseMutex(m_handle) ? true : false; 
		return false;
	}
private:
	mutex_t(const mutex_t&);					// noimpl 
	mutex_t& operator = (const mutex_t&);		// noimpl 
};

/////////////////////////////////////////////////////////////////////////////
class event_t : public handle_t
{
public:
	event_t() { }
	event_t(LPCTSTR lpName, bool bManualReset = false, bool bInitialState = false,
		LPSECURITY_ATTRIBUTES lpEventAttributes = NULL) { 
		create(lpName, bManualReset, bInitialState, lpEventAttributes);
	}
	bool	signal()	{ return m_handle && SetEvent(m_handle) ? true : false; }
	bool	reset()		{ return m_handle && ResetEvent(m_handle) ? true : false;}
	bool	create(LPCTSTR lpName = NULL,bool bManualReset = false,bool bInitialState= false,
				   LPSECURITY_ATTRIBUTES lpEventAttributes = NULL) {
		if (m_handle)
			return false;
		m_handle = CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
		return m_handle != NULL;
	}
	bool wait(DWORD timeout) {
		if (m_handle)
			return WaitForSingleObject(m_handle,timeout) == WAIT_OBJECT_0; 
		return false;
	}
private:
	event_t(const event_t&);					// noimpl 
	event_t& operator = (const event_t&);		// noimpl 
};

/////////////////////////////////////////////////////////////////////////////
// event sequence: takes care of events to be processed in the same order as they were fired 
// from the main thread
typedef std::deque<DWORD>		  event_ids;
typedef event_ids::const_iterator event_id;

class evseq_t : private event_ids {
public:
	evseq_t() { }
	void add(DWORD ev_id) {
		lock_t wait(m_cs_allow);
		if (ev_id == 0)
			 push_front(ev_id);
		else push_back(ev_id);
	}
	bool check(DWORD id) const {
		lock_t wait(m_cs_allow);
		const_iterator iter = std::find(begin(),end(),id);
		return iter  != end();
	}
	void get(std::deque<DWORD>& seq) {
		lock_t wait(m_cs_allow);
		seq.assign(begin(),end());
		clear();
	}
	critsect_t& allow() const { return m_cs_allow; }

protected:
	mutable critsect_t	m_cs_allow;
};
/////////////////////////////////////////////////////////////////////////////
// to start use start() method; thread calls on_execute() method within each 'timeout' interval
// two ways to exit: either use stop() method or on_execute() method should return false
// note: don't use stop() inside on_execute() method
// use thread's allow() method for any synchronization purpose: cslock_t wait(allow()); 
class thread_t : protected handle_t {
public:
	typedef unsigned (__stdcall * start_proc) (void *);
	thread_t(bool multi = false) : m_multi(multi), m_timeout(0) { m_event.create(); }
	~thread_t() { stop(); }
	bool is_started() const { return m_handle != 0; }
	bool start(DWORD timeout)	{
		if (m_handle)
			return true;
		m_timeout = timeout;
		m_handle = (HANDLE)_beginthreadex(NULL,0,thread_proc,this,0,NULL);
		if (!m_handle)
			return false; 
		return true;
	}
	void stop(DWORD timeout=INFINITE) {
		if (!m_handle)
			return;
		m_evseq.add(0); // 0 means exit
		m_event.signal();
		DWORD result = WaitForSingleObject(m_handle, timeout);
		if (result == WAIT_TIMEOUT) // looks like we don't have correct way to exit
			TerminateThread(m_handle,(DWORD)-1);
		close();
	}
	bool in_shutdown() const { return m_evseq.check(0); }
	void fire_event(DWORD id) {
		if (!m_handle || id == 0) // 0 is reserved for exit
			return;
		m_evseq.add(id);
		m_event.signal();
	}
	bool		is_closing() const { m_evseq.check(0); } // see if there is exit event
	critsect_t& allow() const { return m_evseq.allow(); }

protected:
	virtual	bool on_execute()			{ return true; }	
	virtual	void on_event(DWORD /*id*/)	{ } 
	virtual	void on_startup()			{ } 
	virtual	void on_cleanup()			{ } 
	bool check_event() const { 
		DWORD result = WaitForSingleObject(m_event, m_timeout);
		return result == WAIT_OBJECT_0;
	}

	static unsigned __stdcall thread_proc(void* data) {
		thread_t* me = (thread_t*)data;
		HRESULT hr = me->m_multi ?  CoInitializeEx(NULL, COINIT_MULTITHREADED) : CoInitialize(NULL);
		if (FAILED(hr))
			return 0;
		me->on_startup();
		bool exit = false;
		while(!exit) { // till stop is called
			if (me->check_event())  {
				event_ids events;
				me->m_evseq.get(events);
				for (event_id ev = events.begin(); ev != events.end(); ev++) {
					if (*ev == 0)	{	// method stop() has been called
						me->on_cleanup();
						exit = true;
						break;
					}
					else me->on_event(*ev);
				}
			}
			else {
				if (me->m_evseq.check(0))
					continue;			// waiting for shutdown event 
				if (me->on_execute())
					continue;
				me->close();			// false returned	
				exit = true;
			}
		}
		CoUninitialize();
		return 0;
	}

protected:
	event_t				m_event;
private:
	bool				m_multi;
	evseq_t				m_evseq;	// we process events in order they occur
	DWORD				m_timeout;
private:
	thread_t(const thread_t&);					// noimpl 
	thread_t& operator = (const thread_t&);		// noimpl 
};

}; // mtlib
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // _THREAD_H