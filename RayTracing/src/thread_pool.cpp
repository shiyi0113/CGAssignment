#include "thread_pool.h"


void ThreadPool::WorkerThread(ThreadPool* master)
{
	while (master->alive) {
		Task* task = master->getTask();
		if (task != nullptr) {
			task->run();
		}
		else {

		}
	}
}
void ThreadPool::addTask(Task* task)
{
	std::lock_guard<std::mutex> guard(lock);
	tasks.push_back(task);
}

Task* ThreadPool::getTask()
{
	std::lock_guard<std::mutex> guard(lock);
	if(tasks.empty())
		return nullptr;
	Task* task = tasks.front();
	tasks.pop_front();
	return task;
}
