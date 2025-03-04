#pragma once

#include <vector>
#include <thread>
#include <list>
#include <mutex>

class Task {
public:
	virtual void run() = 0;
};

class ThreadPool {
public:
	ThreadPool(size_t thread_count = 0) {
		alive = true;
		if (thread_count == 0) {
			thread_count = std::thread::hardware_concurrency();
		}
		for (size_t i = 0;i < thread_count;i++) {
			threads.push_back(std::thread(WorkerThread, this));
		}
	}
	~ThreadPool() {
		while (!tasks.empty()) {
			std::this_thread::yield();
		}
		alive = false;
		for (auto& thread : threads) {
			thread.join();
		}
		threads.clear();
	}

	static void WorkerThread(ThreadPool* master); // 工作线程
	void addTask(Task* task);    // 添加任务
	Task* getTask();             // 获取任务
private:
	bool alive;
	std::vector<std::thread> threads;
	std::list<Task*> tasks;
	std::mutex lock;
};