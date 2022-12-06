#pragma once
#include <queue>
#include <iostream>
#include <pthread.h>

using callback = void (*)(void *);

class TaskQ
{
public:
	TaskQ()
	{
		function = nullptr;
		arg = nullptr;
	}
	TaskQ(callback f, void * arg)
	{
		this->arg = arg;
		this->function = f;
	}
	~TaskQ()
	{
		function = nullptr;
		arg = nullptr;
	}

	callback function;
	void * arg;
};

class TaskQueue:public TaskQ
{
public:
	TaskQueue();
	~TaskQueue();

	//添加任务
	void AddTask(TaskQ task);
	//取任务
	TaskQ takeTask();
	//当前任务个数
	inline size_t taskNumber()
	{
		return m_taskQ.size();
	}

private:
	std::queue<TaskQ> m_taskQ;
	pthread_mutex_t m_mutex;
};

