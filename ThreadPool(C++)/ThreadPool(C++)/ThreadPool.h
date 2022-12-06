#pragma once
#include "TaskQueue.h"

class ThreadPool
{
public:
	ThreadPool(int min, int max);
	int getLiveNum();
	int getWorkNum();
	void addTask(TaskQ task);
	~ThreadPool();

private:
	void deleteThread();
	static void * manager(void * arg);
	static void * workers(void * arg);

private:
	TaskQueue * m_queue;

	pthread_t m_manager;
	pthread_t * m_workers;
	int m_maxNum;
	int m_minNum;
	int m_liveNum;
	int m_workNum;
	int m_killNum;
	bool m_shutdown;

	pthread_mutex_t m_mutex;
	pthread_cond_t m_notEmpty;
};

