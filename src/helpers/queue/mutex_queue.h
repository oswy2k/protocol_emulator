#ifndef MUTEX_QUEUE_H
#define MUTEX_QUEUE_H

/* Needed include files */
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

/* MACROS */
#define MAX_QUEUE_CAPACITY 128
#define DATA_SIZE 1024

/* Enums */
typedef enum{
	QUEUE_OK = 0,
	QUEUE_ERROR_FULL,
	QUEUE_ERROR_EMPTY
} queue_status_t;

/* Typedefs */
typedef struct _queue_item_t{
	uint16_t size;				//Actual size of data
	uint8_t buffer[1024];		//Data buffer
} queue_item_t;

typedef struct _queue_t{
	queue_item_t items[MAX_QUEUE_CAPACITY];	//Array of queue items

	pthread_mutex_t mutex;					//Mutex for queue

	uint16_t front;							//Front of the queue
	uint16_t rear;							//Rear of the queue
	uint16_t count;							//Number of items in the queue
} queue_t;

/* Public Functions */
void queue_init(queue_t* queue);
queue_status_t queue_enqueue(queue_t* queue, queue_item_t* item);
queue_status_t queue_dequeue(queue_t* queue, queue_item_t* item);
queue_status_t queue_peek(queue_t* queue, queue_item_t* item);
void queue_count(queue_t* queue, uint16_t* count);
bool queue_is_empty(queue_t* queue);
bool queue_is_full(queue_t* queue);

#endif // MUTEX_QUEUE_H