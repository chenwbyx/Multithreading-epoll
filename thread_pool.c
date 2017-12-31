//thread_pool.c

#include "thread_pool.h"
#include "queue.h"
#include <stdlib.h>
#include <pthread.h>

struct thread_pool {
	unsigned int thread_count;
	pthread_t *threads;
	queue_t tasks;
	pthread_mutex_t lock;
	pthread_cond_t task_ready;
};

struct task {
	void* (*routine)(void *arg);
	void *arg;
};

static void cleanup(pthread_mutex_t* lock) {
	pthread_mutex_unlock(lock);
}

static void * worker(thread_pool_t pool) {
	struct task *t;
	while(1) {
		pthread_mutex_lock(&pool->lock);
		pthread_cleanup_push((void(*)(void*))cleanup, &pool->lock);
		while(queue_isempty(pool->tasks)) {
			pthread_cond_wait(&pool->task_ready, &pool->lock);
			/*A  condition  wait  (whether  timed  or  not)  is  a  cancellation point ... a side-effect of acting upon a cancellation request  while in a condition wait is that the mutex is (in  effect)  re-acquired  before  calling  the  first  cancellation  cleanup  handler.*/
		}
		t=(struct task*)queue_dequeue(pool->tasks);
		pthread_cleanup_pop(0);
		pthread_mutex_unlock(&pool->lock);
		t->routine(t->arg);/*todo: report returned value*/
		free(t);
	}
	return NULL;
}

thread_pool_t thread_pool_create(unsigned int thread_count) {
	unsigned int i;
	thread_pool_t pool=NULL;
	pool=(thread_pool_t)malloc(sizeof(struct thread_pool));
	pool->thread_count=thread_count;
	pool->threads=(pthread_t*)malloc(sizeof(pthread_t)*thread_count);
	
	pool->tasks=queue_create();
	
	pthread_mutex_init(&pool->lock, NULL);
	pthread_cond_init(&pool->task_ready, NULL);
	
	for(i=0; i<thread_count; i++) {
		pthread_create(pool->threads+i, NULL, (void*(*)(void*))worker, pool);
	}
	return pool;
}

void thread_pool_add_task(thread_pool_t pool, void* (*routine)(void *arg), void *arg) {
	struct task *t;
	pthread_mutex_lock(&pool->lock);
	t=(struct task*)queue_enqueue(pool->tasks, sizeof(struct task));
	t->routine=routine;
	t->arg=arg;
	pthread_cond_signal(&pool->task_ready);
	pthread_mutex_unlock(&pool->lock);
}

void thread_pool_destroy(thread_pool_t pool) {
	unsigned int i;
	for(i=0; i<pool->thread_count; i++) {
		pthread_cancel(pool->threads[i]);
	}
	for(i=0; i<pool->thread_count; i++) {
		pthread_join(pool->threads[i], NULL);
	}
	pthread_mutex_destroy(&pool->lock);
	pthread_cond_destroy(&pool->task_ready);
	queue_destroy(pool->tasks);
	free(pool->threads);
	free(pool);
}