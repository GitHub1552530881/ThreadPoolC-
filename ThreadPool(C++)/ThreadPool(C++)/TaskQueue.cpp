#include "TaskQueue.h"

TaskQueue::TaskQueue()
{
	pthread_mutex_init(&m_mutex, NULL);
}

TaskQueue::~TaskQueue()
{
	pthread_mutex_destroy(&m_mutex);
}

void TaskQueue::AddTask(TaskQ task)
{
	pthread_mutex_lock(&m_mutex);
	m_taskQ.push(task);
	pthread_mutex_unlock(&m_mutex);
}

TaskQ TaskQueue::takeTask()
{
	TaskQ q;
	pthread_mutex_lock(&m_mutex);
	if (m_taskQ.size() > 0)
	{
		q = m_taskQ.front();
		m_taskQ.pop();
	}
	pthread_mutex_unlock(&m_mutex);
	return q;
}
