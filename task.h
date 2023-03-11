#ifndef __TASK_H__
#define __TASK_H__
#include <functional>
class Task
{
public:
	virtual int run() = 0;
	//std::function<bool()> isExit = nullptr;
};

#endif //__TASK_H__

