#include "thread_pool.h"
#include <assert.h>

static void thread_pool_exit_handler(void* data);
static void* thread_pool_cycle(void* data);

// global variable
static uint_t thread_pool_task_id = 0;
static int debug = 0;

thread_pool_t* thread_pool_init()
{
	int err;

	// 1 allocate memory
	thread_pool_t* tp = new thread_pool_t();
	if (tp == nullptr)
	{
		fprintf(stderr, "thread_pool_init: new failed\n");
		return nullptr;
	}

	// 2 set mutex and cond
	if (thread_mutex_create(&tp->mtx) != THREAD_OK)
	{
		delete tp;
		fprintf(stderr, "thread_pool_init: thread_mutex_create failed\n");
		return nullptr;
	}
	if (thread_cond_create(&tp->cond) != THREAD_OK)
	{
		delete tp;
		(void)thread_mutex_destroy(&tp->mtx);
		fprintf(stderr, "thread_pool_init: thread_cond_create failed\n");
		return nullptr;
	}

	// 3 start each thread and run the thread cycle
	// prepare the attributes to initialize threads
	pthread_attr_t attr;
	err = pthread_attr_init(&attr);
	if (err != 0)
	{
		fprintf(stderr, "pthread_attr_init failed: %s\n", strerror(errno));
		delete tp;
		(void)thread_mutex_destroy(&tp->mtx);
		(void)thread_cond_destroy(&tp->cond);
		return nullptr;
	}

	// detach thread
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (err != 0)
	{
		fprintf(stderr, "pthread_attr_setdetachstate failed: %s\n", strerror(errno));
		delete tp;
		(void)thread_mutex_destroy(&tp->mtx);
		(void)thread_cond_destroy(&tp->cond);
		(void)pthread_attr_destroy(&attr);
		return nullptr;
	}

	// create thread and run thread cycle
	pthread_t tid; 
	for (uint_t n = 0; n < tp->threads; n++)
	{
		err = pthread_create(&tid, &attr, thread_pool_cycle, tp);
		if (err != 0)
		{
			fprintf(stderr, "pthread_create failed: %s\n", strerror(errno));
			delete tp;
			(void)thread_mutex_destroy(&tp->mtx);
			(void)thread_cond_destroy(&tp->cond);
			(void)pthread_attr_destroy(&attr);
			return nullptr;
		}
	}

	// delete the attributes
	(void)pthread_attr_destroy(&attr);
	return tp;
}

static void* thread_pool_cycle(void* data)
{
	thread_pool_t* tp = (thread_pool_t*)data;
	thread_task_t* task = nullptr;

	if (debug) fprintf(stderr, "thread in pool \"%s\" started\n", tp->name);
	while (1)
	{
		// 1 acquire the mutex
		if (thread_mutex_lock(&tp->mtx) != THREAD_OK)
		{
			return nullptr;
		}

		// 2 no tasks in the queue -> wait for the signal
		while (tp->queue.first == nullptr)
		{
			// cond -> unlock mtx -> when get waked ->  require mtx
			if (thread_cond_wait(&tp->cond, &tp->mtx) != THREAD_OK)
			{
				(void)thread_mutex_unlock(&tp->mtx);
				return nullptr;
			}
		}

		// 3 accquire a task from the queue head
		tp->waiting--;
		task = tp->queue.first;
		tp->queue.first = task->next;
		if (tp->queue.first == nullptr)
		{
			tp->queue.last = &tp->queue.first;
		}

		// 4 unlock the mutex
		if (thread_mutex_unlock(&tp->mtx) != THREAD_OK)
		{
			return nullptr;
		}

		// 5 run that task
		task->handler(task->ctx);

		// if it is the termination task it will not return and free the task
		// the termination task is an automation variable 
		// 6 free the task and its parameters
		free(task);
	}
}

thread_task_t* thread_task_alloc(size_t size) // the size of the parameter structure
{
	// 1 allocate the memory of task and its pramaters
	thread_task_t* task = nullptr;
	
	task = (thread_task_t*)calloc(1, sizeof(thread_task_t) + size); // calloc -> allocate memory and initialize it to 0
	if (task == nullptr)
	{
		return nullptr;
	}
	task->ctx = task + 1; // point to the parameters 
	return task;
}

// push the task to the task queue
int_t thread_task_post(thread_pool_t* tp, thread_task_t* task)
{
	// 1 acquire mutex
	if (thread_mutex_lock(&tp->mtx) != THREAD_OK)
	{
		return THREAD_ERROR;
	}

	// 2 if it reachs the maximum -> exit
	if (tp->waiting >= tp->max_queue)
	{
		(void)thread_mutex_unlock(&tp->mtx);
		fprintf(stderr, "thread_pool \"%s\" queue overflow: %ld tasks wating\n", tp->name, tp->waiting);
		return THREAD_ERROR;
	}

	// 3 signal to one of threads waiting 
	// pthread_cond_broadcast(&tp->cond) <-> (thread_cond_signal(&tp->cond)
	if (thread_cond_signal(&tp->cond) != THREAD_OK)
	{
		(void)thread_mutex_unlock(&tp->mtx);
		fprintf(stderr, "%s: thread_cond_signal failed\n", tp->name);
		return THREAD_ERROR;
	}

	// 4 set up remaining task info
	task->id = thread_pool_task_id++;
	task->next = nullptr;

	// 5 push the task to the queue tail
	*(tp->queue.last) = task;
	tp->queue.last = &task->next;

	// 6 update pool info 
	tp->waiting++;

	// 7 release the mutex
	(void)thread_mutex_unlock(&tp->mtx);

	if (debug)
	{
		fprintf(stderr, "thread_pool \"%s\" added a task: %lu\n", tp->name, task->id);
	}
	return THREAD_OK;
}

// post the close thread task to the task queue 
void thread_pool_destroy(thread_pool_t* tp)
{
	volatile uint_t lock; // volatile -> Every access will read the latest value from memory

	// 1 create the task
	thread_task_t task2;
	task2.handler = thread_pool_exit_handler;
	task2.ctx = (void*)&lock;

	for (uint_t n = 0; n < tp->threads; n++)
	{
		lock = 1;
		
		// 2 post the task -> each task close one thread
		if (thread_task_post(tp, &task2) != THREAD_OK)
		{
			return;
		}

		while (lock)
		{
			// if lock is not modified to 0 by thread_pool_exit_handler
			// make the main thread to relinquish (give up) the CPU
			// wait the thread to kill itself and set the lock to 0
			sched_yield();
		}
	}

	// 3 clean up
	(void)thread_mutex_destroy(&tp->mtx);
	(void)thread_cond_destroy(&tp->cond);
	delete tp;
}

// close the thread
static void thread_pool_exit_handler(void* data)
{
	uint_t* lock = (uint_t*)data;
	*lock = 0;
	pthread_exit(0);
}