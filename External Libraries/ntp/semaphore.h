#ifndef __BOOST_SEPATHORE_H__
#define __BOOST_SEPATHORE_H__

#include <thread>
#include <mutex>
#include <condition_variable>

//Implements a semaphore using std primitives.
class semaphore
{
	mutable std::mutex m_mut;
	mutable std::condition_variable m_cond;
    const size_t m_maxCount;
	mutable size_t m_count;
	semaphore& operator = (semaphore&);
	semaphore(semaphore&);
public:

	~semaphore();
	semaphore(size_t maxCount, size_t initialCount ) ;
	static const size_t MAX_COUNT;
	static const size_t WAIT_INFINITY;
	bool wait(size_t timeoutMs) const;
	void wait() const;
	size_t releaseOne() { return release(1); }
	size_t release(size_t amount2Release);
	void releaseAll();
	size_t tryReleaseOne();
	inline size_t getCount() const { return m_count; }
};

#endif
