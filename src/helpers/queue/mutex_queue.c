/* System libraries */
#include <stdlib.h>
#include <string.h>

/* User defined libraries */
#include "mutex_queue.h"

/* Initialize queue after defining it */
void queue_init(queue_t* queue){
	
	/* Set all items to zero */
	queue->front = 0;
	queue->rear = 0;
	queue->count = 0;
	memset(queue->items, 0, sizeof(queue_item_t) * MAX_QUEUE_CAPACITY);


	/* Initialize mutex for later use */
	pthread_mutex_init(&queue->mutex, NULL);
}


/* Push item into queue */
queue_status_t queue_enqueue(queue_t* queue, queue_item_t* item){
	
	/* Guard against full queue */
	if(queue->count >= MAX_QUEUE_CAPACITY){
		return QUEUE_ERROR_FULL;
	}

	pthread_mutex_lock(&queue->mutex);

	/* Put item in rear of queue */
	memcpy(&(queue->items[queue->rear++]), item, sizeof(queue_item_t));

	/* Check if rear of queue is less than max item size */
	if(queue->rear >= MAX_QUEUE_CAPACITY){
		queue->rear = 0;
	}

	/* Increment item count */
	queue->count++;

	pthread_mutex_unlock(&queue->mutex);

	return QUEUE_OK;
}

/* Pop item from queue */
queue_status_t queue_dequeue(queue_t* queue, queue_item_t* item){

	/* Guard against null queue */
	if(queue->count == 0){
		return QUEUE_ERROR_EMPTY;
	}

	pthread_mutex_lock(&queue->mutex);

	/* Dequeue item from front */	
	if(item != NULL){
		memcpy(item,&(queue->items[queue->front++]),sizeof(queue_item_t));
	}
	else{
		queue->front++;
	}

	/* Check if front of queue is less than max item size */
	if(queue->front >= MAX_QUEUE_CAPACITY){
		queue->front = 0;
	}

	/* Decrement item count */
	queue->count--;

	pthread_mutex_unlock(&queue->mutex);

	return QUEUE_OK;
}

queue_status_t queue_peek(queue_t* queue, queue_item_t* item){
	
	/* Guard against null queue */
	if(queue->count == 0){
		return QUEUE_ERROR_EMPTY;
	}

	pthread_mutex_lock(&queue->mutex);

	/* Peek item from front */	
	memcpy(item,&(queue->items[queue->front]),sizeof(queue_item_t));

	pthread_mutex_unlock(&queue->mutex);

	return QUEUE_OK;
}

/* Get item count from queue */
void queue_count(queue_t* queue, uint16_t* count){
	pthread_mutex_lock(&queue->mutex);

	/* Get data while mutex is locked */
	*count = queue->count;	

	pthread_mutex_unlock(&queue->mutex);
}

/* Check if queue is empty */
bool queue_is_empty(queue_t* queue){

	bool is_empty = false;

	pthread_mutex_lock(&queue->mutex);

	if(queue->count == 0){
		is_empty = true;
	}
	else{
		is_empty = false;
	}

	pthread_mutex_unlock(&queue->mutex);

	return is_empty;
}

/* Check if queue is full */
bool queue_is_full(queue_t* queue){

	bool is_full = false;

	pthread_mutex_lock(&queue->mutex);

	if(queue->count == MAX_QUEUE_CAPACITY){
		is_full = true;
	}
	else{
		is_full = false;
	}

	pthread_mutex_unlock(&queue->mutex);

	return is_full;
}