#include "threadpool.h"
#include <iostream>
using namespace std;
void ThreadPool::init(int _num)
{
	unique_lock<mutex> lock(m_mutex);
	m_threadNum = _num;
	cout << "pool Init" << endl;
}

void ThreadPool::start()
{
	unique_lock<mutex> lock(m_mutex);
	if (m_threadNum <= 0)
	{
		cerr << "Please Init Pool" << endl;
		return;
	}
	if (!m_threads.empty())
	{
		cerr << "Pool has started" << endl;
		return;
	}
	for (int i = 0; i < m_threadNum; i++)
	{
		auto th = make_shared<thread>(&ThreadPool::run, this);
		m_threads.emplace_back(th);
	}
}

void ThreadPool::stop()
{
	m_isExit = true;
	m_cv.notify_all();
	for (auto& th : m_threads)
		th->join();
	unique_lock<mutex> lock(m_mutex);
	m_threads.clear();
}

bool ThreadPool::isExit() const
{
	return m_isExit;
}

void ThreadPool::addTask(std::shared_ptr<Task> _task)
{
	unique_lock<mutex> lock(m_mutex);
	/*_task->isExit = [this]
	{
		return isExit();
	};*/
	m_taskLists.emplace_back(_task);
	lock.unlock();
	m_cv.notify_one();
}

std::shared_ptr<Task> ThreadPool::getTask()
{
	unique_lock<mutex> lock(m_mutex);
	if (m_taskLists.empty())
		m_cv.wait(lock);
	if (isExit())
		return nullptr;
	if (!m_taskLists.empty())
	{
		auto task = m_taskLists.front();
		m_taskLists.pop_front();
		return task;
	}
	return nullptr;
}

void ThreadPool::run()
{
	while(!isExit())
	{
		auto task = getTask();
		if (!task)
			continue;
		try
		{
			task->run();
		}
		catch (const std::exception&)
		{

		}
	}
}
