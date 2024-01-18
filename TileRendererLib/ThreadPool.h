#pragma once
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cassert>
#include <list>

class ThreadPool {
public:
	void start(int num = 0);
	void queueJob(const std::function<void(int)>& job);
	void stop();
	bool busy();

	void join();
	size_t size() { return threads.size(); }

private:
	void ThreadLoop(int);

	bool should_terminate = false;           // Tells threads to stop looking for jobs
	std::mutex queue_mutex;                  // Prevents data races to the job queue
	std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination 
	std::vector<std::thread> threads;
	std::atomic<int> num_working = 0;
	std::queue<std::function<void(int)>> jobs;
};