#include "ThreadPool.h"
#include "pch.h"

void ThreadPool::start(int num) {
	uint32_t num_threads; // Max # of threads the system supports
	if (num == 0)
		num_threads = std::thread::hardware_concurrency();
	else
		num_threads = num;
	threads.resize(num_threads);
	for (uint32_t i = 0; i < num_threads; i++) {
		threads.at(i) = std::thread([this, i]()
			{
				ThreadLoop(i);
			});
	}
}

void ThreadPool::ThreadLoop(int i) {
	while (true) {
		std::function<void(int)> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			mutex_condition.wait(lock, [this] {
				return !jobs.empty() || should_terminate;
				});
			if (should_terminate) {
				return;
			}
			job = jobs.front();
			jobs.pop();
			num_working++;
		}

		job(i);

		num_working--;
	}
}

void ThreadPool::queueJob(const std::function<void(int)>& job) {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		jobs.push(job);
	}
	mutex_condition.notify_one();
}

bool ThreadPool::busy() {
	bool poolbusy;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		poolbusy = !jobs.empty() || (num_working > 0);
	}
	return poolbusy;
}

void ThreadPool::join()
{
	while (busy()) {};
}

void ThreadPool::stop() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		should_terminate = true;
	}
	mutex_condition.notify_all();
	for (std::thread& active_thread : threads) {
		active_thread.join();
	}
	threads.clear();
}