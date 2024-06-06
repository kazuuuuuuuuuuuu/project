#include "thread.h"

int thread_mutex_create(pthread_mutex_t* mtx)
{
	int err;
	pthread_mutexattr_t attr;

	err = pthread_mutexattr_init(&attr);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutexattr_init failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutexattr_settype(PTHREAD_MUTEX_ERRORCHECK) failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	err = pthread_mutex_init(mtx, &attr);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	err = pthread_mutexattr_destroy(&attr);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutexattr_destroy failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	return THREAD_OK;
}

int thread_mutex_destroy(pthread_mutex_t* mtx)
{
	int err;
	err = pthread_mutex_destroy(mtx);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutex_destroy failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}
	
	return THREAD_OK;
}

int thread_mutex_lock(pthread_mutex_t* mtx)
{
	int err;
	err = pthread_mutex_lock(mtx);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutex_lock failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	return THREAD_OK;
}

int thread_mutex_unlock(pthread_mutex_t* mtx)
{
	int err;
	err = pthread_mutex_unlock(mtx);
	if (err != 0)
	{
		fprintf(stderr, "pthread_mutex_unlock failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	return THREAD_OK;
}

int thread_cond_create(pthread_cond_t* cond)
{
	int err;
	err = pthread_cond_init(cond, NULL);
	if (err != 0)
	{
		fprintf(stderr, "thread_cond_create failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	return THREAD_OK;
}
int thread_cond_destroy(pthread_cond_t* cond)
{
	int err;
	err = pthread_cond_destroy(cond);
	if (err != 0)
	{
		fprintf(stderr, "thread_cond_destroy failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	return THREAD_OK;
}

int thread_cond_signal(pthread_cond_t* cond)
{
	int err;
	err = pthread_cond_signal(cond);
	if (err != 0)
	{
		fprintf(stderr, "thread_cond_signal failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	return THREAD_OK;
}

int thread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mtx)
{
	int err;
	err = pthread_cond_wait(cond, mtx);
	if (err != 0)
	{
		fprintf(stderr, "thread_cond_wait failed: %s\n", strerror(errno));
		return THREAD_ERROR;
	}

	return THREAD_OK;
}