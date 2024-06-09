#include "manual_event.h"

const size_t manual_event::WAIT_INFINITY = ~size_t(0);

manual_event::manual_event( bool asSignaled )
	: m_isSignaled(asSignaled)
{
}

manual_event::~manual_event()
{}

bool manual_event::wait(size_t timeoutMs) const
{
	const auto wait_dur(WAIT_INFINITY == timeoutMs ? std::chrono::milliseconds::max() : std::chrono::milliseconds(timeoutMs));
	std::unique_lock<std::mutex> lock(m_mut);
	while(!m_isSignaled)
	{
		if( std::cv_status::timeout == m_cond.wait_for(lock, wait_dur))
			return false;//timed out;
	}

	return true;
}

void manual_event::wait() const
{
	std::unique_lock<std::mutex> lock(m_mut); 
	while(!m_isSignaled)
		m_cond.wait(lock);
}

void manual_event::set()
{
	{
		std::lock_guard<std::mutex> lock(m_mut);
		m_isSignaled = true;
	}
	m_cond.notify_all();
}

void manual_event::reset()
{
	{
		std::lock_guard<std::mutex> lock(m_mut);
		m_isSignaled = false;
	}
}

