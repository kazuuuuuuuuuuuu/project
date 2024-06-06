#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#ifdef  __cplusplus
extern "C"
{
#endif //  __cplusplus

#include "thread.h"

constexpr uint_t DEFAULT_THREADS_NUM = 4;
constexpr uint_t DEFAULT_QUEUE_NUM = 65535;

typedef struct thread_task_s thread_task_t;
typedef struct thread_pool_queue_s thread_pool_queue_t;
typedef struct thread_pool_s thread_pool_t;

struct thread_task_s
{
	thread_task_t* next;
	uint_t id;
	void (*handler)(void* data);
	void* ctx;

	thread_task_s() :
		next(nullptr), id(-1), handler(nullptr), ctx(nullptr)
	{
	}
};

struct thread_pool_queue_s
{
	thread_task_t* first;
	thread_task_t** last;

	thread_pool_queue_s() :
		first(nullptr), last(&first)
	{
	}
};

struct thread_pool_s
{
	pthread_mutex_t mtx;
	pthread_cond_t cond;

	thread_pool_queue_t queue;
	uint_t waiting;

	const char* name;
	uint_t threads;
	uint_t max_queue;

	thread_pool_s() :
		queue(), waiting(0), name("default"), threads(DEFAULT_THREADS_NUM), max_queue(DEFAULT_QUEUE_NUM)
	{
	}
};

thread_task_t* thread_task_alloc(size_t size);
int_t thread_task_post(thread_pool_t* tp, thread_task_t* task);
thread_pool_t* thread_pool_init();
void thread_pool_destroy(thread_pool_t* tp);

#ifdef  __cplusplus
}
#endif //  __cplusplus

#endif
