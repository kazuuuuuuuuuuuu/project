#ifndef _THREAD_H_
#define _THREAD_H_

#ifdef  __cplusplus
extern "C"
{
#endif //  __cplusplus

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

	typedef intptr_t int_t;
	typedef uintptr_t uint_t;

#define THREAD_OK 0
#define THREAD_ERROR -1

	// wrapper function
	// mutex
	int thread_mutex_create(pthread_mutex_t* mtx);
	int thread_mutex_destroy(pthread_mutex_t* mtx);
	int thread_mutex_lock(pthread_mutex_t* mtx);
	int thread_mutex_unlock(pthread_mutex_t* mtx);
	// conditional variable
	int thread_cond_create(pthread_cond_t* cond);
	int thread_cond_destroy(pthread_cond_t* cond);
	int thread_cond_signal(pthread_cond_t* cond);
	int thread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mtx);

#ifdef  __cplusplus
}
#endif //  __cplusplus

#endif
