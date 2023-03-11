#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__
#include <mutex>
#include <vector>
#include <thread>
#include <list>
#include "task.h"
class ThreadPool
{
public:
	void init(int _num);
	void start();
	void stop();
	bool isExit() const;
	void addTask(std::shared_ptr<Task> _task);
	std::shared_ptr<Task> getTask();
private:
	void run();
	std::mutex m_mutex;
	std::condition_variable m_cv;
	int m_threadNum = 0;
	std::vector<std::shared_ptr<std::thread>> m_threads;
	std::list<std::shared_ptr<Task>> m_taskLists;
	bool m_isExit = false;

};
#endif //__THREADPOOL_H__

