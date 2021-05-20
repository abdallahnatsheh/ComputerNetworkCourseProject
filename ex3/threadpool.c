/**
 * threadpool.c
 *
 * This file will contain your implementation of a threadpool.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include<stdint.h>
#include "threadpool.h"



/* This function is the work function of the thread */
void* do_work(void* p) {
	threadpool * pool = (threadpool *) p;
	work_t* cur;	//The q element
	

	while(1) {
		pool->qsize = pool->qsize;
		pthread_mutex_lock(&(pool->qlock));  //get the q lock.


		while( pool->qsize == 0) {	//if the size is 0 then wait.  
			if(pool->shutdown) {
				pthread_mutex_unlock(&(pool->qlock));
				pthread_exit(NULL);
			}
			//wait until the condition says its no emtpy and give up the lock. 
			pthread_mutex_unlock(&(pool->qlock));  //get the qlock.
			pthread_cond_wait(&(pool->q_not_empty),&(pool->qlock));

			//check to see if in shutdown mode.
			if(pool->shutdown) {
				pthread_mutex_unlock(&(pool->qlock));
				pthread_exit(NULL);
			}
		}

		cur = pool->qhead;	//set the cur variable.  

		pool->qsize--;		//decriment the size.  

		if(pool->qsize == 0) {
			pool->qhead = NULL;
			pool->qtail = NULL;
		}
		else {
			pool->qhead = cur->next;
		}

		if(pool->qsize == 0 && ! pool->shutdown) {
			//the q is empty again, now signal that its empty.
			pthread_cond_signal(&(pool->q_empty));
		}
		pthread_mutex_unlock(&(pool->qlock));
		(cur->routine) (cur->arg);   //actually do work.
		free(cur);						//free the work storage.  	
	}
}

threadpool* create_threadpool(int num_threads_in_pool) {
  threadpool *pool;
	int i;

  // sanity check the argument
  if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
    return NULL;

  pool = (threadpool *) malloc(sizeof(threadpool));
  if (pool == NULL) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;
  }

  pool->threads = (pthread_t*) malloc (sizeof(pthread_t) * num_threads_in_pool);

  if(!pool->threads) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;	
  }

  pool->num_threads = num_threads_in_pool; //set up structure members
  pool->qsize = 0;
  pool->qhead = NULL;
  pool->qtail = NULL;
  pool->shutdown = 0;
  pool->dont_accept = 0;

  //initialize mutex and condition variables.  
  if(pthread_mutex_init(&pool->qlock,NULL)) {
    fprintf(stderr, "Mutex initiation error!\n");
	return NULL;
  }
  if(pthread_cond_init(&(pool->q_empty),NULL)) {
    fprintf(stderr, "CV initiation error!\n");	
	return NULL;
  }
  if(pthread_cond_init(&(pool->q_not_empty),NULL)) {
    fprintf(stderr, "CV initiation error!\n");	
	return NULL;
  }

  //make threads

  for (i = 0;i < num_threads_in_pool;i++) {
	  if(pthread_create(&(pool->threads[i]),NULL,do_work,pool)) {
	    fprintf(stderr, "Thread initiation error!\n");	
		return NULL;		
	  }
  }
  return (threadpool*) pool;
}


void dispatch(threadpool* from_me, dispatch_fn dispatch_to_here, void *arg) {
  threadpool *pool = (threadpool *) from_me;
	work_t * cur;
	

	//make a work queue element.  
	cur = (work_t*) malloc(sizeof(work_t));
	if(cur == NULL) {
		fprintf(stderr, "Out of memory creating a work struct!\n");
		return;	
	}

	cur->routine = dispatch_to_here;
	cur->arg = arg;
	cur->next = NULL;

	pthread_mutex_lock(&(pool->qlock));

	if(pool->dont_accept) { //Just incase someone is trying to queue more
		free(cur); //work structs.  
		return;
	}
	if(pool->qsize == 0) {
		pool->qhead = cur;  //set to only one
		pool->qtail = cur;
		pthread_cond_signal(&(pool->q_not_empty));  //I am not empty.  
	} else {
		pool->qtail->next = cur;	//add to end;
		pool->qtail = cur;			
	}
	pool->qsize++;
	pthread_mutex_unlock(&(pool->qlock));  //unlock the queue.
}

void destroy_threadpool(threadpool* destroyme) {
	threadpool *pool = (threadpool *) destroyme;
	void* nothing;
	int i = 0;

	pthread_mutex_lock(&(pool->qlock));
	pool->dont_accept = 1;
	while(pool->qsize != 0) {
		pthread_cond_wait(&(pool->q_empty),&(pool->qlock));  //wait until the q is empty.
	}
	pool->shutdown = 1;  //allow shutdown
	pthread_cond_broadcast(&(pool->q_not_empty));  //allow code to return NULL;
	pthread_mutex_unlock(&(pool->qlock));

	//kill everything.  
	for(;i < pool->num_threads;i++) {
		pthread_cond_broadcast(&(pool->q_not_empty));
//allowcode to return NULL;/
		pthread_join(pool->threads[i],&nothing);
	}

	free(pool->threads);

	pthread_mutex_destroy(&(pool->qlock));
	pthread_cond_destroy(&(pool->q_empty));
	pthread_cond_destroy(&(pool->q_not_empty));
	return;
}

int dispatch_to_me(void *arg) {
  int seconds = (intptr_t) arg;
  fprintf(stdout, "  in dispatch %d\n", seconds);
  fprintf(stdout, "  done dispatch %d\n", seconds);
  return 0;
}

/*
int main(int argc, char *argv[]) { 
	threadpool *tp;
  int i = 0;
    tp = create_threadpool(5);
  for(;i < 5;i ++) {
	  dispatch(tp, dispatch_to_me, (void *) i);		
  }
  fprintf(stdout, "\n**main** done first\n");
  sleep(1);
  fprintf(stdout, "\n\n");
  
  destroy_threadpool(tp);
  
  exit(-1);
}*/