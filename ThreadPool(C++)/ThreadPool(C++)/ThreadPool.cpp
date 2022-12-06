#include "ThreadPool.h"
#include <string.h>
#include <iostream>
using namespace std;
#include <unistd.h>

ThreadPool::ThreadPool(int min, int max)
{
	do 
	{
		m_queue = new TaskQueue;
		if (m_queue == nullptr)
		{
			cout << "TaskQueue分配失败！\n" << endl;
			break;
		}
		m_killNum = 0;
		m_liveNum = min;
		m_minNum = min;
		m_maxNum = max;
		m_workNum = 0;
		m_shutdown = false;

		m_workers = new pthread_t[max];
		if (m_workers == nullptr)
		{
			cout << "workers分配失败！\n" << endl;
			break;
		}
		memset(m_workers, 0, sizeof(pthread_t)*m_maxNum);

		if (pthread_mutex_init(&m_mutex, NULL) != 0 ||
			pthread_cond_init(&m_notEmpty, NULL) != 0)
		{
			cout << "互斥量和信号量初始化失败！\n" << endl;
			break;
		}

		pthread_create(&m_manager, NULL, manager, this);
		for (int i = 0;i < min; i++)
		{
			pthread_create(&m_workers[i], NULL, workers, this);
			cout << "创建子线程, ID: " << to_string(m_workers[i]) << endl;
		}
	} while (0);

}

ThreadPool::~ThreadPool()
{
	m_shutdown = true;
	pthread_join(m_manager, NULL);
	for (int i = 0; i < m_maxNum; i++)
	{
		pthread_cond_signal(&m_notEmpty);
	}
	if (m_queue) delete m_queue;
	if (m_workers)delete[] m_workers;
	pthread_cond_destroy(&m_notEmpty);
	pthread_mutex_destroy(&m_mutex);
}

int ThreadPool::getLiveNum()
{
	pthread_mutex_lock(&m_mutex);
	int liveNum = m_liveNum;
	pthread_mutex_unlock(&m_mutex);
	return liveNum;
}

int ThreadPool::getWorkNum()
{
	pthread_mutex_lock(&m_mutex);
	int workNum = m_workNum;
	pthread_mutex_unlock(&m_mutex);
	return workNum;
}

void ThreadPool::deleteThread()
{
	pthread_t tid = pthread_self();
	for (int i = 0; i < m_maxNum;i++)
	{
		if (m_workers[i] == tid)
		{
			cout << "线程" << tid << "成功销毁" << endl;
			m_workers[i] = 0;
			break;
		}
	}
	pthread_exit(NULL);
}

void ThreadPool::addTask(TaskQ task)
{
	if (m_shutdown)
	{
		return;
	}
	m_queue->AddTask(task);
	pthread_cond_signal(&m_notEmpty);
}

void * ThreadPool::manager(void * arg)
{
	ThreadPool * pool = static_cast<ThreadPool*>(arg);
	while (!pool->m_shutdown)
	{
		sleep(3);
		int liveNum = pool->getLiveNum();
		int workNum = pool->getWorkNum();
		size_t queueNum = pool->m_queue->taskNumber();
		int NUMBER = 2;

		//添加
		if (liveNum < queueNum && liveNum < pool->m_maxNum)
		{
			int count = 0;
			pthread_mutex_lock(&pool->m_mutex);
			for (int i = 0; i < pool->m_maxNum && count < NUMBER && liveNum < pool->m_maxNum;i++)
			{
				if (pool->m_workers[i] == 0)
				{
					pthread_create(&pool->m_workers[i], NULL, workers, pool);
					count++;
				}
			}
			pthread_mutex_unlock(&pool->m_mutex);
		}

		//删除
		if (workNum * 2 < liveNum && liveNum > pool->m_minNum)
		{
			pthread_mutex_lock(&pool->m_mutex);
			pool->m_killNum = NUMBER;
			pthread_mutex_unlock(&pool->m_mutex);
			for (int i = 0; i < pool->m_killNum && liveNum > pool->m_minNum;i++)
			{
				pthread_cond_signal(&pool->m_notEmpty);
			}
		}
	}
	return nullptr;
}

void * ThreadPool::workers(void * arg)
{
	ThreadPool * pool = static_cast<ThreadPool*>(arg);//强制类型转换
	while (1)
	{
		pthread_mutex_lock(&pool->m_mutex);
		size_t queueNum = pool->m_queue->taskNumber();
		while(queueNum == 0 && !pool->m_shutdown)
		{
			pthread_cond_wait(&pool->m_notEmpty,&pool->m_mutex);
			if (pool->m_killNum > 0 && pool->m_liveNum > pool->m_minNum)
			{
				pool->m_liveNum--;
				pool->m_killNum--;
				pool->deleteThread();
				pthread_mutex_unlock(&pool->m_mutex);
			}
		}
		if (pool->m_shutdown)
		{
			pool->deleteThread();
			pthread_mutex_unlock(&pool->m_mutex);
		}

		TaskQ q = pool->m_queue->takeTask();
		pool->m_workNum++;
		pthread_mutex_unlock(&pool->m_mutex);
		cout << "thread " << to_string(pthread_self()) << " start working..." << endl;

		q.function(q.arg);
		delete q.arg;
		q.arg = nullptr;

		cout << "thread " << to_string(pthread_self()) << " end working..." << endl;
		pthread_mutex_lock(&pool->m_mutex);
		pool->m_workNum--;
		pthread_mutex_unlock(&pool->m_mutex);
	}
	return nullptr;
}
