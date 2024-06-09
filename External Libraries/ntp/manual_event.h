#ifndef __MANUAL_EVENT_H__
#define __MANUAL_EVENT_H__

#include <thread>
#include <mutex>
#include <condition_variable>

//Implements a windows like, manual reset object, with std primitives
class manual_event
{
	mutable std::mutex m_mut;
	mutable std::condition_variable m_cond;
	bool m_isSignaled;

	manual_event& operator = (manual_event&);
	manual_event(manual_event&);
public:

	static const size_t WAIT_INFINITY;

	~manual_event();
	manual_event( bool asSignaled ) ;
	bool wait(size_t timeoutMs) const;
	void wait() const;
	void set();
	void reset();
};

#endif//__MANUAL_EVENT_H__
