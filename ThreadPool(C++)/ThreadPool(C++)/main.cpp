#include <iostream>
using namespace std;
#include <unistd.h>
#include "ThreadPool.h"

void function(void * arg)
{
	int num = *(int*)arg;
	cout << "线程" << pthread_self() << "正在工作，num = " << num << endl;
	sleep(1);
}

int main()
{
	ThreadPool pool(3, 10);
	for (int i = 0; i < 100; i++)
	{
		int * num = new int(i);
		pool.addTask(TaskQ(function,num));
	}
	sleep(20);
    return 0;
}