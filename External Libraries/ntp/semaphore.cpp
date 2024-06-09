#include "semaphore.h"
#include <assert.h>
#include <algorithm>

const size_t semaphore::MAX_COUNT = ~size_t(0);
const size_t semaphore::WAIT_INFINITY = ~size_t(0);

semaphore::semaphore(size_t maxCount, size_t initialCount )
	: m_maxCount(maxCount), m_count(initialCount)
{
	assert( m_maxCount >= m_count );		
}

semaphore::~semaphore()	{}

bool semaphore::wait(size_t timeoutMs) const
{
	const auto wait_dur(WAIT_INFINITY == timeoutMs ? std::chrono::milliseconds::max() : std::chrono::milliseconds(timeoutMs));
	std::unique_lock<std::mutex> lock(m_mut);
	while(!m_count)
	{
		if (std::cv_status::timeout == m_cond.wait_for(lock, wait_dur))
			return false;//timed out;
	}
	if(m_count)
		--m_count;

	return true;
}

void semaphore::wait() const
{
	std::unique_lock<std::mutex> lock(m_mut);
	while(!m_count)
		m_cond.wait(lock);
	
	if(m_count)
		--m_count;
}	

size_t semaphore::tryReleaseOne()
{
	bool notify = true;
	size_t prevCount = 0;
	{
		std::lock_guard<std::mutex> lock(m_mut);
		if (m_maxCount > m_count)
			prevCount = m_count++;
		else
			notify = false;//Current counter already at it's maximal value, notification is not required.
	}

	if(notify)
		m_cond.notify_one();

	return prevCount;
}

void semaphore::releaseAll()
{
	{
		std::lock_guard<std::mutex> lock(m_mut);
		m_count=m_maxCount;
	}
	m_cond.notify_all();
}

size_t semaphore::release(size_t amount2Release)
{
	size_t prevCount;
	{
		std::lock_guard<std::mutex> lock(m_mut);
		prevCount = m_count;
		amount2Release = std::min(amount2Release, m_maxCount - m_count);
		m_count+= amount2Release;
	}

	for(size_t i=0; i < amount2Release; ++i)
		m_cond.notify_one();

	return prevCount;
}

