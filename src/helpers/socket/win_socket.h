#ifndef SOCKET_WIN_H
#define SOCKET_WIN_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <pthread.h>

#include "../queue/mutex_queue.h"

/* enums */
typedef enum{
	SOCKET_STATUS_UNINITIALIZED = 0,
	SOCKET_STATUS_INITIALIZED,
	SOCKET_STATUS_CONNECTED,
	SOCKET_STATUS_DISCONNECTED
} socket_status_t;

typedef enum{
	SOCKET_ERROR_NONE = 0,
	SOCKET_ERROR_ALREADY_INITIALIZED,
	SOCKET_ERROR_DLL_STARTUP,
	SOCKET_ERROR_HOST_NOT_FOUND,
	SOCKET_ERROR_INVALID_CONFIGURATION,
	SOCKET_ERROR_CONNECTION_FAILED,

	SOCKET_ERROR_NO_DATA,
	SOCKET_ERROR_QUEUE_FULL
} socket_error_t;

typedef enum{
	SOCKET_BLOCK=0,
	SOCKET_NONBLOCK
} socket_read_t;

/* Typedefs */
typedef struct _socket_t{
	WSADATA socket_data;		//Socket data structure
	PHOSTENT host_ptr;			//Host structure with Info for connection
	SOCKADDR_IN socket_addr;	//Socket address structure for connection
	SOCKET connection_socket;	//Socket for connection handling
	struct{
		pthread_t read;	//Thread for reading from socket
		pthread_t write;	//Thread for writing to socket
	}threads;

	struct{
		queue_t read;		//Queue for reading from socket
		queue_t write;	//Queue for writing to socket
	}queues;

	socket_status_t status;				//Socket active flag	
} socket_t;

/* Public Functions */
void socket_get_hostname(char* hostname);
socket_error_t socket_init(socket_t *socket);
socket_error_t socket_connect(socket_t *socket, const char *host, const int port);
socket_error_t socket_disconnect(socket_t *socket);
socket_status_t socket_is_connected(socket_t *socket);

socket_error_t socket_peek(socket_t *socket, queue_item_t *item);
socket_error_t socket_delete(socket_t *socket);
socket_error_t socket_read(socket_t *socket, queue_item_t *item, socket_read_t block);
socket_error_t socket_write(socket_t *socket, queue_item_t *item, socket_read_t block);

#endif //SOCKET_WIN_H